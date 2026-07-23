package emulator

import (
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"syscall"
	"time"
	"unsafe"

	"google.golang.org/protobuf/proto"

	pb "DrunkPCDebugger/generated"
)

const (
	tiocgptn  = 0x80045430
	tiocsptlk = 0x40045431
	flashSize = 65536
)

type Emulator struct {
	PortPath    string
	master      *os.File
	flash       []byte
	busAcquired bool
}

func Start() (*Emulator, error) {
	master, portPath, err := openPty()
	if err != nil {
		return nil, err
	}
	emulator := &Emulator{
		PortPath: portPath,
		master:   master,
		flash:    make([]byte, flashSize),
	}
	go emulator.serve()
	return emulator, nil
}

func (e *Emulator) Close() {
	e.master.Close()
}

func openPty() (*os.File, string, error) {
	master, err := os.OpenFile("/dev/ptmx", os.O_RDWR|syscall.O_NOCTTY, 0)
	if err != nil {
		return nil, "", err
	}
	var ptyNumber uint32
	if err := ptyIoctl(master.Fd(), tiocgptn, unsafe.Pointer(&ptyNumber)); err != nil {
		master.Close()
		return nil, "", err
	}
	var unlock int32
	if err := ptyIoctl(master.Fd(), tiocsptlk, unsafe.Pointer(&unlock)); err != nil {
		master.Close()
		return nil, "", err
	}
	return master, fmt.Sprintf("/dev/pts/%d", ptyNumber), nil
}

func ptyIoctl(fd uintptr, request uintptr, arg unsafe.Pointer) error {
	_, _, errno := syscall.Syscall(syscall.SYS_IOCTL, fd, request, uintptr(arg))
	if errno != 0 {
		return errno
	}
	return nil
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
	frame, err := e.readFrame()
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
	return e.writeFrame(responseData)
}

func (e *Emulator) handle(request *pb.RpcRequest) *pb.RpcResponse {
	response := &pb.RpcResponse{RequestId: request.RequestId}
	switch payload := request.Payload.(type) {
	case *pb.RpcRequest_AcquireBus:
		response.Payload = &pb.RpcResponse_AcquireBus{AcquireBus: e.acquireBus()}
	case *pb.RpcRequest_ReleaseBus:
		response.Payload = &pb.RpcResponse_ReleaseBus{ReleaseBus: e.releaseBus()}
	case *pb.RpcRequest_WriteFlash:
		response.Payload = &pb.RpcResponse_WriteFlash{WriteFlash: e.writeFlash(payload.WriteFlash)}
	case *pb.RpcRequest_ReadFlash:
		response.Payload = &pb.RpcResponse_ReadFlash{ReadFlash: e.readFlash(payload.ReadFlash)}
	}
	return response
}

func (e *Emulator) acquireBus() *pb.ErrorResponse {
	e.busAcquired = true
	return okStatus()
}

func (e *Emulator) releaseBus() *pb.ErrorResponse {
	e.busAcquired = false
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

func (e *Emulator) readFrame() ([]byte, error) {
	header := make([]byte, 2)
	if err := e.readFull(header); err != nil {
		return nil, err
	}
	payload := make([]byte, binary.LittleEndian.Uint16(header))
	if err := e.readFull(payload); err != nil {
		return nil, err
	}
	return payload, nil
}

func (e *Emulator) writeFrame(payload []byte) error {
	frame := make([]byte, 2+len(payload))
	binary.LittleEndian.PutUint16(frame, uint16(len(payload)))
	copy(frame[2:], payload)
	_, err := e.master.Write(frame)
	return err
}

func (e *Emulator) readFull(buf []byte) error {
	for filled := 0; filled < len(buf); {
		n, err := e.master.Read(buf[filled:])
		if err != nil {
			if isSlaveClosed(err) {
				time.Sleep(10 * time.Millisecond)
				continue
			}
			return err
		}
		filled += n
	}
	return nil
}

func isSlaveClosed(err error) bool {
	var pathErr *os.PathError
	return errors.As(err, &pathErr) && pathErr.Err == syscall.EIO
}
