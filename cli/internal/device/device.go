package device

import (
	"fmt"
	"io"

	"go.uber.org/zap"
	"google.golang.org/protobuf/proto"

	pb "drunkpc-debugger/generated"
	"drunkpc-debugger/internal/progress"
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

type Transport interface {
	WriteFrame(payload []byte) error
	ReadFrame() ([]byte, error)
}

type Device struct {
	transport     Transport
	logger        *zap.SugaredLogger
	out           io.Writer
	nextRequestID uint32
}

func New(t Transport, logger *zap.SugaredLogger, out io.Writer) *Device {
	return &Device{
		transport:     t,
		logger:        logger,
		out:           out,
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
	if err := genericError(response); err != nil {
		return nil, err
	}
	if response.RequestId != request.RequestId {
		return nil, fmt.Errorf("response id mismatch: sent %d, got %d",
			request.RequestId, response.RequestId)
	}
	return response, nil
}

func genericError(response *pb.RpcResponse) error {
	generic := response.GetGeneric()
	if generic == nil {
		return nil
	}
	if err := statusToError(generic); err != nil {
		return err
	}
	return fmt.Errorf("device returned generic response with OK status")
}

func (d *Device) AcquireBus() error {
	return d.busControl(true)
}

func (d *Device) ReleaseBus() error {
	return d.busControl(false)
}

func (d *Device) busControl(acquire bool) error {
	response, err := d.call(&pb.RpcRequest{
		Payload: &pb.RpcRequest_BusControlRequest{
			BusControlRequest: &pb.BusControlRequest{Acquire: acquire},
		},
	})
	if err != nil {
		return err
	}
	return statusToError(response.GetBusControlRequest())
}

func (d *Device) PowerOn() error {
	return d.setPower(true)
}

func (d *Device) PowerOff() error {
	return d.setPower(false)
}

func (d *Device) setPower(enable bool) error {
	response, err := d.call(&pb.RpcRequest{
		Payload: &pb.RpcRequest_PowerOnBusRequest{
			PowerOnBusRequest: &pb.PowerOnBusRequest{Enable: enable},
		},
	})
	if err != nil {
		return err
	}
	return statusToError(response.GetPowerOnBusRequest())
}

func (d *Device) WriteFlash(data []byte, address uint32) error {
	total := uint32(len(data))
	if total == 0 {
		return nil
	}

	bar := progress.NewBytes(d.out, "writing flash", total)
	for offset := uint32(0); offset < total; offset += MaxChunkSize {
		chunk := data[offset:min(offset+MaxChunkSize, total)]
		if err := d.writeChunk(address+offset, chunk); err != nil {
			bar.Abort()
			return err
		}
		bar.Advance(uint32(len(chunk)))
	}
	bar.Finish()
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
	if size == 0 {
		return data, nil
	}

	bar := progress.NewBytes(d.out, "reading flash", size)
	for offset := uint32(0); offset < size; offset += MaxChunkSize {
		chunkSize := min(MaxChunkSize, size-offset)
		chunk, err := d.readChunk(address+offset, chunkSize)
		if err != nil {
			bar.Abort()
			return nil, err
		}
		data = append(data, chunk...)
		bar.Advance(chunkSize)
	}
	bar.Finish()
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
