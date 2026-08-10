package progress

import (
	"fmt"
	"io"
	"time"

	"github.com/schollz/progressbar/v3"
)

const (
	barWidth      = 30
	renderTimeout = 65 * time.Millisecond
)

type Bar struct {
	bar *progressbar.ProgressBar
}

func NewBytes(out io.Writer, description string, total uint32) *Bar {
	return &Bar{bar: progressbar.NewOptions64(
		int64(total),
		progressbar.OptionSetWriter(out),
		progressbar.OptionSetDescription(description),
		progressbar.OptionSetTheme(progressbar.ThemeASCII),
		progressbar.OptionSetWidth(barWidth),
		progressbar.OptionThrottle(renderTimeout),
		progressbar.OptionShowBytes(true),
		progressbar.OptionShowTotalBytes(true),
		progressbar.OptionShowCount(),
		progressbar.OptionSetRenderBlankState(true),
		progressbar.OptionOnCompletion(func() { fmt.Fprintln(out) }),
	)}
}

func (b *Bar) Advance(count uint32) {
	b.bar.Add64(int64(count))
}

func (b *Bar) Finish() {
	b.bar.Finish()
}

func (b *Bar) Abort() {
	b.bar.Exit()
}
