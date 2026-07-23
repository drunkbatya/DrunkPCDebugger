package device_test

import (
	"bytes"
	"crypto/rand"
	"errors"
	"runtime"
	"testing"

	"go.uber.org/zap"

	pb "DrunkPCDebugger/generated"
	"DrunkPCDebugger/internal/device"
	"DrunkPCDebugger/internal/emulator"
	"DrunkPCDebugger/internal/transport"
)

func startDevice(t *testing.T) *device.Device {
	t.Helper()
	if runtime.GOOS != "linux" {
		t.Skip("pty emulator requires linux")
	}
	emu, err := emulator.Start()
	if err != nil {
		t.Fatalf("failed to start emulator: %v", err)
	}
	t.Cleanup(emu.Close)

	logger := zap.NewNop().Sugar()
	serial, err := transport.NewSerial(emu.PortPath, 115200, logger)
	if err != nil {
		t.Fatalf("failed to open emulator port: %v", err)
	}
	t.Cleanup(serial.Close)

	return device.New(serial, logger)
}

func acquireBus(t *testing.T, dev *device.Device) {
	t.Helper()
	if err := dev.AcquireBus(); err != nil {
		t.Fatalf("failed to acquire bus: %v", err)
	}
}

func randomData(t *testing.T, size int) []byte {
	t.Helper()
	data := make([]byte, size)
	if _, err := rand.Read(data); err != nil {
		t.Fatalf("failed to generate test data: %v", err)
	}
	return data
}

func requireDeviceError(t *testing.T, err error, want pb.ErrType) {
	t.Helper()
	var deviceErr *device.DeviceError
	if !errors.As(err, &deviceErr) {
		t.Fatalf("expected DeviceError, got: %v", err)
	}
	if deviceErr.Type != want {
		t.Fatalf("expected %v, got %v", want, deviceErr.Type)
	}
}

func TestAcquireAndReleaseBus(t *testing.T) {
	dev := startDevice(t)
	acquireBus(t, dev)
	if err := dev.ReleaseBus(); err != nil {
		t.Fatalf("failed to release bus: %v", err)
	}
}

func TestWriteAndReadFlash(t *testing.T) {
	dev := startDevice(t)
	acquireBus(t, dev)

	written := randomData(t, 3000)
	if err := dev.WriteFlash(written, 0x100); err != nil {
		t.Fatalf("write failed: %v", err)
	}

	read, err := dev.ReadFlash(0x100, 3000)
	if err != nil {
		t.Fatalf("read failed: %v", err)
	}
	if !bytes.Equal(written, read) {
		t.Fatalf("read data differs from written data")
	}
}

func TestWriteFlashSingleByte(t *testing.T) {
	dev := startDevice(t)
	acquireBus(t, dev)

	if err := dev.WriteFlash([]byte{0x42}, 0); err != nil {
		t.Fatalf("write failed: %v", err)
	}
	read, err := dev.ReadFlash(0, 1)
	if err != nil {
		t.Fatalf("read failed: %v", err)
	}
	if read[0] != 0x42 {
		t.Fatalf("expected 0x42, got 0x%02x", read[0])
	}
}

func TestWriteFlashOutOfRange(t *testing.T) {
	dev := startDevice(t)
	acquireBus(t, dev)

	err := dev.WriteFlash(randomData(t, 2000), 0xFF00)
	requireDeviceError(t, err, pb.ErrType_ERROR_TYPE_OUT_OF_RANGE)
}

func TestReadFlashOutOfRange(t *testing.T) {
	dev := startDevice(t)
	acquireBus(t, dev)

	_, err := dev.ReadFlash(0xFF00, 2000)
	requireDeviceError(t, err, pb.ErrType_ERROR_TYPE_OUT_OF_RANGE)
}

func TestWriteFlashWithoutBus(t *testing.T) {
	dev := startDevice(t)

	err := dev.WriteFlash(randomData(t, 100), 0)
	requireDeviceError(t, err, pb.ErrType_ERROR_TYPE_BUS_NOT_ACQUIRED)
}

func TestReadFlashWithoutBus(t *testing.T) {
	dev := startDevice(t)

	_, err := dev.ReadFlash(0, 100)
	requireDeviceError(t, err, pb.ErrType_ERROR_TYPE_BUS_NOT_ACQUIRED)
}
