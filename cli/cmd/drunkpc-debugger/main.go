package main

import (
	"fmt"
	"log"
	"os"

	"drunkpc-debugger/internal/args"
	"drunkpc-debugger/internal/console"
	"drunkpc-debugger/internal/device"
	"drunkpc-debugger/internal/logging"
	"drunkpc-debugger/internal/transport"
	"drunkpc-debugger/internal/version"
)

func main() {
	os.Exit(run())
}

func run() int {
	parsedArgs := args.Parse()

	if parsedArgs.OnlyShowVersion {
		fmt.Printf("%s\n", version.GetPrintableString())
		return 0
	}

	loggerCore, err := logging.NewAppLogger(parsedArgs.LogLevel)
	if err != nil {
		log.Printf("failed to init logger: %v", err)
		return 1
	}
	defer loggerCore.Sync()
	logger := loggerCore.Sugar()

	serial, err := transport.NewSerial(parsedArgs.Port, parsedArgs.Baudrate, logger)
	if err != nil {
		logger.Errorf("%v", err)
		return 1
	}
	defer serial.Close()

	session := console.NewSession(device.New(serial, logger, os.Stdout), os.Stdout, logger)
	registry := console.DefaultCommands()

	if len(parsedArgs.Commands) > 0 {
		if err := console.RunBatch(registry, session, parsedArgs.Commands); err != nil {
			logger.Errorf("%v", err)
			return 1
		}
	}

	if parsedArgs.Interactive || len(parsedArgs.Commands) == 0 {
		if err := console.RunShell(registry, session); err != nil {
			logger.Errorf("%v", err)
			return 1
		}
	}
	return 0
}
