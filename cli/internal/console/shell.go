package console

import (
	"errors"
	"io"
	"strings"

	"github.com/chzyer/readline"
)

var ErrStopShell = errors.New("stop shell")

func RunShell(r *Registry, s *Session) error {
	listener := &questionListener{registry: r}
	editor, err := readline.NewEx(&readline.Config{
		Prompt:       "drunkpc :-> ",
		AutoComplete: buildCompleter(r),
		Listener:     listener,
	})
	if err != nil {
		return err
	}
	defer editor.Close()
	listener.out = editor.Stdout()

	for {
		line, err := editor.Readline()
		if errors.Is(err, readline.ErrInterrupt) {
			continue
		}
		if errors.Is(err, io.EOF) {
			return nil
		}
		if err != nil {
			return err
		}
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		if stop := runShellLine(r, s, line); stop {
			return nil
		}
	}
}

func runShellLine(r *Registry, s *Session, line string) bool {
	err := r.Dispatch(s, line)
	if errors.Is(err, ErrStopShell) {
		return true
	}
	if err != nil {
		s.Printf("error: %v\n", err)
	}
	return false
}
