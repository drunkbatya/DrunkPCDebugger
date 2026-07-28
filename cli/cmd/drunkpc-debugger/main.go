package main

import (
	"drunkpc-debugger/internal/args"
	"drunkpc-debugger/internal/device"
	"drunkpc-debugger/internal/logging"
	"drunkpc-debugger/internal/transport"
	"drunkpc-debugger/internal/version"
	"fmt"
	"log"
	"os"
)

func main() {
	os.Exit(run())
}

func run() int {
	parsedArgs := args.Parse()

	loggerCore, err := logging.NewAppLogger(parsedArgs.LogLevel)
	if err != nil {
		log.Printf("failed to init logger: %v", err)
		return 1
	}
	defer loggerCore.Sync()
	logger := loggerCore.Sugar()

	serial, transportErr := transport.NewSerial(parsedArgs.Port, parsedArgs.Baudrate, logger)
	if transportErr == nil {
		defer serial.Close()
	}

	if parsedArgs.OnlyShowVersion {
		printVersion(transportErr == nil)
		return 0
	}

	if transportErr != nil {
		logger.Errorf("%v", transportErr)
		return 1
	}

	if err := dispatch(device.New(serial, logger), parsedArgs); err != nil {
		logger.Errorf("%v", err)
		return 1
	}
	return 0
}

func printVersion(deviceConnected bool) {
	cliVersion := version.GetPrintableString()
	fmt.Printf("Tool:\n\t%s\n", cliVersion)

	if deviceConnected {
		deviceVersion := "2.0.0"
		fmt.Printf("Device:\n\t%s\n", deviceVersion)
	}
}

func dispatch(dev *device.Device, parsedArgs args.Args) error {
	switch {
	case parsedArgs.AcquireBus != nil:
		return dev.AcquireBus()
	case parsedArgs.ReleaseBus != nil:
		return dev.ReleaseBus()
	case parsedArgs.PowerOn != nil:
		return dev.PowerOn()
	case parsedArgs.PowerOff != nil:
		return dev.PowerOff()
	case parsedArgs.WriteFlash != nil:
		return writeFlashFromFile(dev, parsedArgs.WriteFlash)
	case parsedArgs.ReadFlash != nil:
		return readFlashToFile(dev, parsedArgs.ReadFlash)
	}
	return nil
}

func writeFlashFromFile(dev *device.Device, cmd *args.WriteFlashCmd) error {
	data, err := os.ReadFile(cmd.SourceFile)
	if err != nil {
		return err
	}
	return dev.WriteFlash(data, cmd.Address)
}

func readFlashToFile(dev *device.Device, cmd *args.ReadFlashCmd) error {
	data, err := dev.ReadFlash(cmd.Address, cmd.Size)
	if err != nil {
		return err
	}
	return os.WriteFile(cmd.TargetFile, data, 0644)
}
