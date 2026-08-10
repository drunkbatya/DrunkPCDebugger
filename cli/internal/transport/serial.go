package transport

import (
	"fmt"
	"strings"
	"time"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
	"go.uber.org/zap"
)

const (
	readTimeout  = 3 * time.Second
	serialMarker = "_dpcdbg_"
)

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
	logger.Infof("serial port isn't specified, trying to found it..")
	ports, err := enumerator.GetDetailedPortsList()
	if err != nil {
		return "", fmt.Errorf("failed to get serial ports list :-(( : %v", err)
	}

	found := drunkpcPorts(ports)
	switch len(found) {
	case 0:
		return "", fmt.Errorf("no attached devices found :-(")
	case 1:
		logger.Infof("found device :-) at path: %s", found[0])
		return found[0], nil
	default:
		return "", fmt.Errorf("found %d devices :-)) : [%s], please pick only one with -p",
			len(found), strings.Join(found, ", "))
	}
}

func drunkpcPorts(ports []*enumerator.PortDetails) []string {
	found := make([]string, 0, len(ports))
	for _, port := range ports {
		if isDrunkpcPort(port) {
			found = append(found, port.Name)
		}
	}
	return found
}

func isDrunkpcPort(port *enumerator.PortDetails) bool {
	return strings.Contains(port.SerialNumber, serialMarker) ||
		strings.Contains(port.Name, serialMarker)
}

func (s *SerialTransport) Close() {
	if s.port != nil {
		s.port.Close()
	}
}
