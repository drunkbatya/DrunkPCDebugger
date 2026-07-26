package transport

import (
	"encoding/binary"
	"fmt"
	"io"
)

const maxFrameSize = 4096

type Framer struct {
	reader io.Reader
	writer io.Writer
}

func NewFramer(rw io.ReadWriter) *Framer {
	return &Framer{reader: rw, writer: rw}
}

func (f *Framer) WriteFrame(payload []byte) error {
	if len(payload) > maxFrameSize {
		return fmt.Errorf("frame too big: %d bytes (max %d)", len(payload), maxFrameSize)
	}
	frame := make([]byte, 2+len(payload))
	binary.LittleEndian.PutUint16(frame, uint16(len(payload)))
	copy(frame[2:], payload)
	return f.writeFull(frame)
}

func (f *Framer) ReadFrame() ([]byte, error) {
	header := make([]byte, 2)
	if err := f.readFull(header); err != nil {
		return nil, fmt.Errorf("failed to read frame header: %w", err)
	}
	length := binary.LittleEndian.Uint16(header)
	if length > maxFrameSize {
		return nil, fmt.Errorf("incoming frame too big: %d bytes (max %d)", length, maxFrameSize)
	}
	payload := make([]byte, length)
	if err := f.readFull(payload); err != nil {
		return nil, fmt.Errorf("failed to read frame payload: %w", err)
	}
	return payload, nil
}

func (f *Framer) writeFull(data []byte) error {
	for len(data) > 0 {
		n, err := f.writer.Write(data)
		if err != nil {
			return fmt.Errorf("write failed: %w", err)
		}
		data = data[n:]
	}
	return nil
}

func (f *Framer) readFull(buf []byte) error {
	for filled := 0; filled < len(buf); {
		n, err := f.reader.Read(buf[filled:])
		if err != nil {
			return fmt.Errorf("read failed: %w", err)
		}
		if n == 0 {
			return fmt.Errorf("read timeout (got %d of %d bytes)", filled, len(buf))
		}
		filled += n
	}
	return nil
}
