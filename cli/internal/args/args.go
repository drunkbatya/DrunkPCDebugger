package args

import (
	"github.com/alexflint/go-arg"
)

type Args struct {
	Commands    []string `arg:"-c,--command,separate" help:"command to run against the device, may be repeated"`
	Interactive bool     `arg:"-i,--interactive" help:"drop into the interactive shell after running commands"`

	OnlyShowVersion bool   `arg:"-v,--version"`
	Port            string `arg:"-p,--port"`
	Baudrate        int    `arg:"-b,--baud" default:"115200"`
	LogLevel        string `arg:"--log_level" default:"info"`
}

func (Args) Description() string {
	return "DrunkPC flash/debug tool"
}

func Parse() Args {
	var parsed Args
	arg.MustParse(&parsed)
	return parsed
}
