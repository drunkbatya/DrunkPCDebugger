package args

import (
	"github.com/alexflint/go-arg"
)

type Args struct {
	OnlyShowVersion bool   `arg:"-v,--version"`
	Port            string `arg:"-p,--port"`
	Baudrate        int    `arg:"-b,--baud" default: 115200`
	LogLevel        string `arg:"--log_level", default:"info"`
}

func Parse() Args {
	var parsed Args
	arg.MustParse(&parsed)
	return parsed
}
