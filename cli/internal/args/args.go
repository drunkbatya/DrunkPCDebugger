package args

import (
	"github.com/alexflint/go-arg"
)

type Mode string
const (
	WRITE Mode = "write"
	READ Mode = "read"
)

type Args struct {
	OnlyShowVersion bool   `arg:"-v,--version"`
	Port            string `arg:"-p,--port"`
	Baudrate        int    `arg:"-b,--baud" default: 115200`
	LogLevel        string `arg:"--log_level", default:"info"`
	Mode Mode
}

func Parse() Args {
	var parsed Args
	arg.MustParse(&parsed)
	return parsed
}
