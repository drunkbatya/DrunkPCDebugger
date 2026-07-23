package transport

import (
	"encoding/binary"
	"fmt"
	"time"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
	"go.uber.org/zap"
)

const (
	maxFrameSize = 4096
	readTimeout  = 3 * time.Second
)

type SerialTransport struct {
	port   serial.Port
	logger *zap.SugaredLogger
}

func NewSerial(portPath string, baudrate int, logger *zap.SugaredLogger) (*SerialTransport, error) {
	mode := &serial.Mode{
		BaudRate: baudrate,
		DataBits: 8,
		Parity:   serial.NoParity,
		StopBits: serial.OneStopBit,
	}

	if portPath == "" {
		logger.Infow("port isn't set explicitly, trying to found it..")
		ports, err := enumerator.GetDetailedPortsList()
		if err != nil {
			return nil, fmt.Errorf("failed to get serial ports list: %v", err)
		}
		if len(ports) == 0 {
			return nil, fmt.Errorf("no serial ports found :-(")
		}
		return nil, fmt.Errorf("not implemented :-(")
		//for _, port := range ports {
		//	fmt.Printf("Found port: %s\n", port.Name)
		//	if port.IsUSB {
		//		fmt.Printf("   USB ID     %s:%s\n", port.VID, port.PID)
		//		fmt.Printf("   USB serial %s\n", port.SerialNumber)
		//	}
		//}
	}

	port, err := serial.Open(portPath, mode)
	if err != nil {
		return nil, err
	}
	if err := port.SetReadTimeout(readTimeout); err != nil {
		port.Close()
		return nil, fmt.Errorf("failed to set read timeout: %v", err)
	}
	return &SerialTransport{port: port, logger: logger}, nil
}

func (s *SerialTransport) WriteFrame(payload []byte) error {
	if len(payload) > maxFrameSize {
		return fmt.Errorf("frame too big: %d bytes (max %d)", len(payload), maxFrameSize)
	}
	frame := make([]byte, 2+len(payload))
	binary.LittleEndian.PutUint16(frame, uint16(len(payload)))
	copy(frame[2:], payload)
	return s.writeFull(frame)
}

func (s *SerialTransport) ReadFrame() ([]byte, error) {
	header := make([]byte, 2)
	if err := s.readFull(header); err != nil {
		return nil, fmt.Errorf("failed to read frame header: %w", err)
	}
	length := binary.LittleEndian.Uint16(header)
	if length > maxFrameSize {
		return nil, fmt.Errorf("incoming frame too big: %d bytes (max %d)", length, maxFrameSize)
	}
	payload := make([]byte, length)
	if err := s.readFull(payload); err != nil {
		return nil, fmt.Errorf("failed to read frame payload: %w", err)
	}
	return payload, nil
}

func (s *SerialTransport) writeFull(data []byte) error {
	for len(data) > 0 {
		n, err := s.port.Write(data)
		if err != nil {
			return fmt.Errorf("serial write failed: %w", err)
		}
		data = data[n:]
	}
	return nil
}

func (s *SerialTransport) readFull(buf []byte) error {
	for filled := 0; filled < len(buf); {
		n, err := s.port.Read(buf[filled:])
		if err != nil {
			return fmt.Errorf("serial read failed: %w", err)
		}
		if n == 0 {
			return fmt.Errorf("timeout after %v (got %d of %d bytes)", readTimeout, filled, len(buf))
		}
		filled += n
	}
	return nil
}

func (s *SerialTransport) Close() {
	if s.port != nil {
		s.port.Close()
	}
}
