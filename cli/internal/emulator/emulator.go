package emulator

import (
	"io"

	"google.golang.org/protobuf/proto"

	pb "drunkpc-debugger/generated"
	"drunkpc-debugger/internal/transport"
)

const flashSize = 65536

type Emulator struct {
	framer      *transport.Framer
	flash       []byte
	busAcquired bool
	powered     bool
}

func Start(conn io.ReadWriter) *Emulator {
	emulator := &Emulator{
		framer: transport.NewFramer(conn),
		flash:  make([]byte, flashSize),
	}
	go emulator.serve()
	return emulator
}

func (e *Emulator) serve() {
	for {
		request, err := e.readRequest()
		if err != nil {
			return
		}
		if err := e.writeResponse(e.handle(request)); err != nil {
			return
		}
	}
}

func (e *Emulator) readRequest() (*pb.RpcRequest, error) {
	frame, err := e.framer.ReadFrame()
	if err != nil {
		return nil, err
	}
	request := &pb.RpcRequest{}
	if err := proto.Unmarshal(frame, request); err != nil {
		return nil, err
	}
	return request, nil
}

func (e *Emulator) writeResponse(response *pb.RpcResponse) error {
	responseData, err := proto.Marshal(response)
	if err != nil {
		return err
	}
	return e.framer.WriteFrame(responseData)
}

func (e *Emulator) handle(request *pb.RpcRequest) *pb.RpcResponse {
	response := &pb.RpcResponse{RequestId: request.RequestId}
	switch payload := request.Payload.(type) {
	case *pb.RpcRequest_BusControlRequest:
		response.Payload = &pb.RpcResponse_BusControlRequest{BusControlRequest: e.busControl(payload.BusControlRequest)}
	case *pb.RpcRequest_PowerOnBusRequest:
		response.Payload = &pb.RpcResponse_PowerOnBusRequest{PowerOnBusRequest: e.setPower(payload.PowerOnBusRequest)}
	case *pb.RpcRequest_WriteFlash:
		response.Payload = &pb.RpcResponse_WriteFlash{WriteFlash: e.writeFlash(payload.WriteFlash)}
	case *pb.RpcRequest_ReadFlash:
		response.Payload = &pb.RpcResponse_ReadFlash{ReadFlash: e.readFlash(payload.ReadFlash)}
	default:
		response.Payload = &pb.RpcResponse_Generic{Generic: errStatus(pb.ErrType_ERROR_TYPE_BAD_REQUEST)}
	}
	return response
}

func (e *Emulator) busControl(request *pb.BusControlRequest) *pb.ErrorResponse {
	e.busAcquired = request.Acquire
	return okStatus()
}

func (e *Emulator) setPower(request *pb.PowerOnBusRequest) *pb.ErrorResponse {
	e.powered = request.Enable
	return okStatus()
}

func (e *Emulator) writeFlash(request *pb.WriteFlashRequest) *pb.ErrorResponse {
	if !e.busAcquired {
		return errStatus(pb.ErrType_ERROR_TYPE_BUS_NOT_ACQUIRED)
	}
	if int(request.Address)+len(request.Data) > len(e.flash) {
		return errStatus(pb.ErrType_ERROR_TYPE_OUT_OF_RANGE)
	}
	copy(e.flash[request.Address:], request.Data)
	return okStatus()
}

func (e *Emulator) readFlash(request *pb.ReadFlashRequest) *pb.ReadFlashResponse {
	if !e.busAcquired {
		return &pb.ReadFlashResponse{Status: errStatus(pb.ErrType_ERROR_TYPE_BUS_NOT_ACQUIRED)}
	}
	if int(request.Address)+int(request.Size) > len(e.flash) {
		return &pb.ReadFlashResponse{Status: errStatus(pb.ErrType_ERROR_TYPE_OUT_OF_RANGE)}
	}
	return &pb.ReadFlashResponse{
		Status: okStatus(),
		Data:   e.flash[request.Address : request.Address+request.Size],
	}
}

func okStatus() *pb.ErrorResponse {
	return &pb.ErrorResponse{ErrType: pb.ErrType_ERROR_TYPE_OK}
}

func errStatus(errType pb.ErrType) *pb.ErrorResponse {
	return &pb.ErrorResponse{ErrType: errType}
}
