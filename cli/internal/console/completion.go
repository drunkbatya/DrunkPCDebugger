package console

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"github.com/chzyer/readline"
)

func buildCompleter(r *Registry) *readline.PrefixCompleter {
	items := make([]readline.PrefixCompleterInterface, 0, len(r.order))
	for _, name := range r.order {
		cmd := r.byName[name]
		items = append(items, readline.PcItem(cmd.Name, buildArgItems(r, cmd.Args)...))
	}
	return readline.NewPrefixCompleter(items...)
}

func buildArgItems(r *Registry, args []ArgSpec) []readline.PrefixCompleterInterface {
	if len(args) == 0 {
		return nil
	}
	children := buildArgItems(r, args[1:])
	candidates := completionCandidates(r, args[0].Complete)
	return []readline.PrefixCompleterInterface{readline.PcItemDynamic(candidates, children...)}
}

func completionCandidates(r *Registry, complete Completion) readline.DynamicCompleteFunc {
	switch complete {
	case CompleteFile:
		return fileCandidates
	case CompleteCommand:
		return func(string) []string { return r.order }
	default:
		return noCandidates
	}
}

func noCandidates(string) []string {
	return nil
}

func fileCandidates(line string) []string {
	prefix := currentToken(line)
	dir := filepath.Dir(prefix)
	entries, err := os.ReadDir(dir)
	if err != nil {
		return nil
	}
	matches := make([]string, 0, len(entries))
	for _, entry := range entries {
		candidate := entry.Name()
		if dir != "." {
			candidate = filepath.Join(dir, entry.Name())
		}
		if entry.IsDir() {
			candidate += string(filepath.Separator)
		}
		matches = append(matches, candidate)
	}
	return matches
}

func currentToken(line string) string {
	if strings.HasSuffix(line, " ") {
		return ""
	}
	fields := strings.Fields(line)
	if len(fields) == 0 {
		return ""
	}
	return fields[len(fields)-1]
}

type questionListener struct {
	registry *Registry
	out      io.Writer
}

func (l *questionListener) OnChange(line []rune, pos int, key rune) ([]rune, int, bool) {
	if key != '?' {
		return line, pos, false
	}
	stripped, newPos := removeRuneBefore(line, pos)
	fmt.Fprintf(l.out, "\r\n%s\n", contextHelp(l.registry, string(stripped[:newPos])))
	return stripped, newPos, true
}

func removeRuneBefore(line []rune, pos int) ([]rune, int) {
	index := pos - 1
	if index < 0 || index >= len(line) || line[index] != '?' {
		return line, pos
	}
	stripped := make([]rune, 0, len(line)-1)
	stripped = append(stripped, line[:index]...)
	stripped = append(stripped, line[index+1:]...)
	return stripped, index
}

func contextHelp(r *Registry, line string) string {
	fields := strings.Fields(line)
	trailingSpace := line == "" || strings.HasSuffix(line, " ")

	if len(fields) == 0 || (len(fields) == 1 && !trailingSpace) {
		prefix := ""
		if len(fields) == 1 {
			prefix = fields[0]
		}
		return commandListHelp(r, prefix)
	}

	cmd, ok := r.Lookup(fields[0])
	if !ok {
		return fmt.Sprintf("unknown command %q", fields[0])
	}
	argIndex := len(fields) - 1
	if !trailingSpace {
		argIndex--
	}
	if argIndex >= 0 && argIndex < len(cmd.Args) && cmd.Args[argIndex].Complete == CompleteCommand {
		return commandListHelp(r, currentToken(line))
	}
	return argHelp(cmd, argIndex)
}

func commandListHelp(r *Registry, prefix string) string {
	var builder strings.Builder
	for _, name := range r.order {
		if !strings.HasPrefix(name, prefix) {
			continue
		}
		cmd := r.byName[name]
		fmt.Fprintf(&builder, "  %-14s %s\n", cmd.Name, cmd.Summary)
	}
	if builder.Len() == 0 {
		return "no matching commands"
	}
	return strings.TrimRight(builder.String(), "\n")
}

func argHelp(cmd *Command, index int) string {
	if index < 0 || index >= len(cmd.Args) {
		return fmt.Sprintf("%s: no more arguments\n  usage: %s", cmd.Name, cmd.usage())
	}
	arg := cmd.Args[index]
	kind := completionKindName(arg.Complete)
	if arg.Optional {
		return fmt.Sprintf("  [%s]  %s (%s, optional, default %q)", arg.Name, arg.Help, kind, arg.Default)
	}
	return fmt.Sprintf("  <%s>  %s (%s)", arg.Name, arg.Help, kind)
}

func completionKindName(complete Completion) string {
	switch complete {
	case CompleteFile:
		return "file"
	case CompleteCommand:
		return "command"
	default:
		return "value"
	}
}
