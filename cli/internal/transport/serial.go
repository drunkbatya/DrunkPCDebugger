package transport

import (
	"fmt"
	"time"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
	"go.uber.org/zap"
)

const readTimeout = 3 * time.Second

type SerialTransport struct {
	*Framer
	port serial.Port
}

func NewSerial(portPath string, baudrate int, logger *zap.SugaredLogger) (*SerialTransport, error) {
	if portPath == "" {
		resolved, err := autodetectPort(logger)
		if err != nil {
			return nil, err
		}
		portPath = resolved
	}

	port, err := openPort(portPath, baudrate)
	if err != nil {
		return nil, err
	}
	return &SerialTransport{Framer: NewFramer(port), port: port}, nil
}

func openPort(portPath string, baudrate int) (serial.Port, error) {
	mode := &serial.Mode{
		BaudRate: baudrate,
		DataBits: 8,
		Parity:   serial.NoParity,
		StopBits: serial.OneStopBit,
	}
	port, err := serial.Open(portPath, mode)
	if err != nil {
		return nil, err
	}
	if err := port.SetReadTimeout(readTimeout); err != nil {
		port.Close()
		return nil, fmt.Errorf("failed to set read timeout: %v", err)
	}
	return port, nil
}

func autodetectPort(logger *zap.SugaredLogger) (string, error) {
	logger.Infow("port isn't set explicitly, trying to found it..")
	ports, err := enumerator.GetDetailedPortsList()
	if err != nil {
		return "", fmt.Errorf("failed to get serial ports list: %v", err)
	}
	if len(ports) == 0 {
		return "", fmt.Errorf("no serial ports found :-(")
	}
	// TODO: pick the DrunkPC device by USB VID/PID instead of failing
	return "", fmt.Errorf("not implemented :-(")
}

func (s *SerialTransport) Close() {
	if s.port != nil {
		s.port.Close()
	}
}
