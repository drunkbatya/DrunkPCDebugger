package args

import (
	"github.com/alexflint/go-arg"
)

type WriteFlashCmd struct {
	SourceFile string `arg:"positional,required" help:"file to write"`
	Address    uint32 `arg:"positional,required" help:"target flash address (0x... supported)"`
}

type ReadFlashCmd struct {
	TargetFile string `arg:"positional,required" help:"file to save into"`
	Address    uint32 `arg:"positional,required" help:"source flash address (0x... supported)"`
	Size       uint32 `arg:"positional,required" help:"number of bytes to read"`
}

type AcquireBusCmd struct{}

type ReleaseBusCmd struct{}

type PowerOnCmd struct{}

type PowerOffCmd struct{}

type Args struct {
	WriteFlash *WriteFlashCmd `arg:"subcommand:write_flash" help:"write a file into flash at the given address"`
	ReadFlash  *ReadFlashCmd  `arg:"subcommand:read_flash" help:"read flash into a file"`
	AcquireBus *AcquireBusCmd `arg:"subcommand:acquire_bus" help:"take control of the target bus"`
	ReleaseBus *ReleaseBusCmd `arg:"subcommand:release_bus" help:"release the target bus"`
	PowerOn    *PowerOnCmd    `arg:"subcommand:power_on" help:"enable target board power"`
	PowerOff   *PowerOffCmd   `arg:"subcommand:power_off" help:"disable target board power"`

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
	parser := arg.MustParse(&parsed)
	if parser.Subcommand() == nil && !parsed.OnlyShowVersion {
		parser.Fail("missing subcommand")
	}
	return parsed
}
