package console

import (
	"bytes"
	"fmt"
	"os"
	"strconv"
)

func DefaultCommands() *Registry {
	r := NewRegistry()
	r.Add(&Command{Name: "acquire_bus", Summary: "take control of the target bus", Usage: "acquire_bus", Run: runAcquireBus})
	r.Add(&Command{Name: "release_bus", Summary: "release the target bus", Usage: "release_bus", Run: runReleaseBus})
	r.Add(&Command{Name: "power_on", Summary: "enable target board power", Usage: "power_on", Run: runPowerOn})
	r.Add(&Command{Name: "power_off", Summary: "disable target board power", Usage: "power_off", Run: runPowerOff})
	r.Add(&Command{Name: "write_flash", Summary: "write a file into flash", Example: "write_flash firmware.bin 0x0000", Args: []ArgSpec{
		{Name: "file", Help: "file to write", Complete: CompleteFile},
		{Name: "address", Help: "target flash address (0x... supported)"},
	}, Run: runWriteFlash})
	r.Add(&Command{Name: "read_flash", Summary: "read flash into a file", Example: "read_flash dump.bin 0x0000 4096", Args: []ArgSpec{
		{Name: "file", Help: "file to save into", Complete: CompleteFile},
		{Name: "address", Help: "source flash address (0x... supported)", Optional: true, Default: "0"},
		{Name: "size", Help: "number of bytes to read", Optional: true, Default: "32768"},
	}, Run: runReadFlash})
	r.Add(&Command{Name: "verify_flash", Summary: "compare flash against a file", Example: "verify_flash firmware.bin 0x0000", Args: []ArgSpec{
		{Name: "file", Help: "file to compare against", Complete: CompleteFile},
		{Name: "address", Help: "flash address (0x... supported)"},
	}, Run: runVerifyFlash})
	r.Add(&Command{Name: "help", Summary: "list commands or show usage for one", Args: []ArgSpec{
		{Name: "command", Help: "command to describe", Complete: CompleteCommand, Optional: true},
	}, Run: r.help})
	r.Add(&Command{Name: "exit", Summary: "leave the interactive shell", Usage: "exit", Interactive: true, Run: runExit})
	r.Add(&Command{Name: "quit", Summary: "leave the interactive shell", Usage: "quit", Interactive: true, Run: runExit})
	return r
}

func runExit(s *Session, args []string) error {
	return ErrStopShell
}

func runAcquireBus(s *Session, args []string) error {
	return s.device.AcquireBus()
}

func runReleaseBus(s *Session, args []string) error {
	return s.device.ReleaseBus()
}

func runPowerOn(s *Session, args []string) error {
	return s.device.PowerOn()
}

func runPowerOff(s *Session, args []string) error {
	return s.device.PowerOff()
}

func runWriteFlash(s *Session, args []string) error {
	data, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	address, err := parseUint32(args[1], "address")
	if err != nil {
		return err
	}
	if err := s.device.WriteFlash(data, address); err != nil {
		return err
	}
	s.Printf("wrote %d bytes at 0x%08x\n", len(data), address)
	return nil
}

func runReadFlash(s *Session, args []string) error {
	address, err := parseUint32(args[1], "address")
	if err != nil {
		return err
	}
	size, err := parseUint32(args[2], "size")
	if err != nil {
		return err
	}
	data, err := s.device.ReadFlash(address, size)
	if err != nil {
		return err
	}
	if err := os.WriteFile(args[0], data, 0644); err != nil {
		return err
	}
	s.Printf("read %d bytes from 0x%08x into %s\n", len(data), address, args[0])
	return nil
}

func runVerifyFlash(s *Session, args []string) error {
	expected, err := os.ReadFile(args[0])
	if err != nil {
		return err
	}
	address, err := parseUint32(args[1], "address")
	if err != nil {
		return err
	}
	actual, err := s.device.ReadFlash(address, uint32(len(expected)))
	if err != nil {
		return err
	}
	if !bytes.Equal(expected, actual) {
		return fmt.Errorf("verify failed at 0x%08x: contents differ", address)
	}
	s.Printf("verify ok: %d bytes match at 0x%08x\n", len(expected), address)
	return nil
}

func parseUint32(text string, name string) (uint32, error) {
	value, err := strconv.ParseUint(text, 0, 32)
	if err != nil {
		return 0, fmt.Errorf("invalid %s %q: %w", name, text, err)
	}
	return uint32(value), nil
}
