package device

import (
	"fmt"

	"go.uber.org/zap"
	"google.golang.org/protobuf/proto"

	pb "DrunkPCDebugger/generated"
	"DrunkPCDebugger/internal/transport"
)

const MaxChunkSize = uint32(pb.Const_CONST_MAX_CHUNK_SIZE)

type DeviceError struct {
	Type pb.ErrType
	Msg  string
}

func (e *DeviceError) Error() string {
	if e.Msg != "" {
		return fmt.Sprintf("device reported %s: %s", e.Type, e.Msg)
	}
	return fmt.Sprintf("device reported %s", e.Type)
}

func statusToError(status *pb.ErrorResponse) error {
	if status == nil {
		return fmt.Errorf("malformed response: missing or mismatched payload")
	}
	if status.ErrType == pb.ErrType_ERROR_TYPE_OK {
		return nil
	}
	return &DeviceError{Type: status.ErrType, Msg: status.GetMsg()}
}

type Device struct {
	transport     *transport.SerialTransport
	logger        *zap.SugaredLogger
	nextRequestID uint32
}

func New(t *transport.SerialTransport, logger *zap.SugaredLogger) *Device {
	return &Device{
		transport:     t,
		logger:        logger,
		nextRequestID: 1,
	}
}

func (d *Device) call(request *pb.RpcRequest) (*pb.RpcResponse, error) {
	request.RequestId = d.nextRequestID
	d.nextRequestID++

	requestData, err := proto.Marshal(request)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}
	if err := d.transport.WriteFrame(requestData); err != nil {
		return nil, err
	}

	responseData, err := d.transport.ReadFrame()
	if err != nil {
		return nil, err
	}
	response := &pb.RpcResponse{}
	if err := proto.Unmarshal(responseData, response); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}
	if response.RequestId != request.RequestId {
		return nil, fmt.Errorf("response id mismatch: sent %d, got %d",
			request.RequestId, response.RequestId)
	}
	return response, nil
}

func (d *Device) AcquireBus() error {
	response, err := d.call(&pb.RpcRequest{
		Payload: &pb.RpcRequest_AcquireBus{AcquireBus: &pb.AcquireBusRequest{}},
	})
	if err != nil {
		return err
	}
	return statusToError(response.GetAcquireBus())
}

func (d *Device) ReleaseBus() error {
	response, err := d.call(&pb.RpcRequest{
		Payload: &pb.RpcRequest_ReleaseBus{ReleaseBus: &pb.ReleaseBusRequest{}},
	})
	if err != nil {
		return err
	}
	return statusToError(response.GetReleaseBus())
}

func (d *Device) WriteFlash(data []byte, address uint32) error {
	total := uint32(len(data))
	for offset := uint32(0); offset < total; offset += MaxChunkSize {
		chunk := data[offset:min(offset+MaxChunkSize, total)]
		if err := d.writeChunk(address+offset, chunk); err != nil {
			return err
		}
		d.logger.Infof("written %d/%d bytes", offset+uint32(len(chunk)), total)
	}
	return nil
}

func (d *Device) writeChunk(address uint32, chunk []byte) error {
	response, err := d.call(&pb.RpcRequest{
		Payload: &pb.RpcRequest_WriteFlash{WriteFlash: &pb.WriteFlashRequest{
			Address: address,
			Data:    chunk,
		}},
	})
	if err == nil {
		err = statusToError(response.GetWriteFlash())
	}
	if err != nil {
		return fmt.Errorf("write chunk at 0x%08x: %w", address, err)
	}
	return nil
}

func (d *Device) ReadFlash(address uint32, size uint32) ([]byte, error) {
	data := make([]byte, 0, size)
	for offset := uint32(0); offset < size; offset += MaxChunkSize {
		chunkSize := min(MaxChunkSize, size-offset)
		chunk, err := d.readChunk(address+offset, chunkSize)
		if err != nil {
			return nil, err
		}
		data = append(data, chunk...)
		d.logger.Infof("read %d/%d bytes", offset+chunkSize, size)
	}
	return data, nil
}

func (d *Device) readChunk(address uint32, size uint32) ([]byte, error) {
	response, err := d.call(&pb.RpcRequest{
		Payload: &pb.RpcRequest_ReadFlash{ReadFlash: &pb.ReadFlashRequest{
			Address: address,
			Size:    size,
		}},
	})
	if err != nil {
		return nil, fmt.Errorf("read chunk at 0x%08x: %w", address, err)
	}
	readFlash := response.GetReadFlash()
	if readFlash == nil {
		return nil, fmt.Errorf("read chunk at 0x%08x: malformed response", address)
	}
	if err := statusToError(readFlash.Status); err != nil {
		return nil, fmt.Errorf("read chunk at 0x%08x: %w", address, err)
	}
	if uint32(len(readFlash.Data)) != size {
		return nil, fmt.Errorf("read chunk at 0x%08x: expected %d bytes, got %d",
			address, size, len(readFlash.Data))
	}
	return readFlash.Data, nil
}
