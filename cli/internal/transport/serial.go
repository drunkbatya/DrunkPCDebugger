package transport

import (
	"fmt"
	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
	"go.uber.org/zap"
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
	return &SerialTransport{port: port}, nil
}

func (s *SerialTransport) Close() {
	if s.port != nil {
		s.port.Close()
	}
}
