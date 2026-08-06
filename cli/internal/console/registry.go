package console

import (
	"fmt"
	"strings"
)

type Registry struct {
	byName map[string]*Command
	order  []string
}

func NewRegistry() *Registry {
	return &Registry{byName: make(map[string]*Command)}
}

func (r *Registry) Add(cmd *Command) {
	r.byName[cmd.Name] = cmd
	r.order = append(r.order, cmd.Name)
}

func (r *Registry) Lookup(name string) (*Command, bool) {
	cmd, ok := r.byName[name]
	return cmd, ok
}

func (r *Registry) Dispatch(s *Session, line string) error {
	return r.dispatch(s, line, true)
}

func (r *Registry) DispatchBatch(s *Session, line string) error {
	return r.dispatch(s, line, false)
}

func (r *Registry) dispatch(s *Session, line string, allowInteractive bool) error {
	fields := strings.Fields(line)
	if len(fields) == 0 {
		return nil
	}
	cmd, ok := r.Lookup(fields[0])
	if !ok {
		return fmt.Errorf("unknown command %q", fields[0])
	}
	if cmd.Interactive && !allowInteractive {
		return fmt.Errorf("%s: interactive-only command, not available in batch", cmd.Name)
	}
	args := fields[1:]
	if err := checkArgCount(cmd, len(args)); err != nil {
		return err
	}
	return cmd.Run(s, cmd.fillDefaults(args))
}

func checkArgCount(cmd *Command, count int) error {
	if count < cmd.requiredArgs() {
		return fmt.Errorf("%s: too few args, usage: %s", cmd.Name, cmd.usage())
	}
	if count > cmd.maxArgs() {
		return fmt.Errorf("%s: too many args, usage: %s", cmd.Name, cmd.usage())
	}
	return nil
}

func (r *Registry) help(s *Session, args []string) error {
	if len(args) >= 1 && args[0] != "" {
		cmd, ok := r.Lookup(args[0])
		if !ok {
			return fmt.Errorf("unknown command %q", args[0])
		}
		s.Printf("%s\n    %s\n", cmd.usage(), cmd.Summary)
		if cmd.Example != "" {
			s.Printf("    example: %s\n", cmd.Example)
		}
		return nil
	}
	for _, name := range r.order {
		cmd := r.byName[name]
		s.Printf("  %-14s %s\n", cmd.Name, cmd.Summary)
	}
	return nil
}
