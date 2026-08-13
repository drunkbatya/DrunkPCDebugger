package console

import (
	"fmt"
	"io"
	"strings"

	"go.uber.org/zap"

	"drunkpc-debugger/internal/device"
)

type Session struct {
	device *device.Device
	out    io.Writer
	logger *zap.SugaredLogger
}

func NewSession(dev *device.Device, out io.Writer, logger *zap.SugaredLogger) *Session {
	return &Session{device: dev, out: out, logger: logger}
}

func (s *Session) Printf(format string, args ...any) {
	fmt.Fprintf(s.out, format, args...)
}

type Completion int

const (
	CompleteNone Completion = iota
	CompleteFile
	CompleteCommand
)

type ArgSpec struct {
	Name     string
	Help     string
	Complete Completion
	Optional bool
	Default  string
}

type Command struct {
	Name        string
	Summary     string
	Usage       string
	Example     string
	Args        []ArgSpec
	Interactive bool
	Run         func(s *Session, args []string) error
}

func (c *Command) usage() string {
	if len(c.Args) == 0 {
		return c.Usage
	}
	parts := make([]string, 0, len(c.Args)+1)
	parts = append(parts, c.Name)
	for _, arg := range c.Args {
		if arg.Optional {
			parts = append(parts, "["+arg.Name+"]")
		} else {
			parts = append(parts, "<"+arg.Name+">")
		}
	}
	return strings.Join(parts, " ")
}

func (c *Command) requiredArgs() int {
	for i, arg := range c.Args {
		if arg.Optional {
			return i
		}
	}
	return len(c.Args)
}

func (c *Command) maxArgs() int {
	return len(c.Args)
}

func (c *Command) fillDefaults(args []string) []string {
	if len(args) >= len(c.Args) {
		return args
	}
	filled := make([]string, 0, len(c.Args))
	filled = append(filled, args...)
	for i := len(args); i < len(c.Args); i++ {
		filled = append(filled, c.Args[i].Default)
	}
	return filled
}
