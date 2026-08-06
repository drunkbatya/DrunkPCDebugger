package console

import (
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"
)

func TestContextHelpCommandList(t *testing.T) {
	r := DefaultCommands()
	help := contextHelp(r, "")
	if !strings.Contains(help, "write_flash") || !strings.Contains(help, "acquire_bus") {
		t.Fatalf("expected command list, got:\n%s", help)
	}
}

func TestContextHelpCommandPrefix(t *testing.T) {
	r := DefaultCommands()
	help := contextHelp(r, "wr")
	if !strings.Contains(help, "write_flash") {
		t.Fatalf("expected write_flash, got:\n%s", help)
	}
	if strings.Contains(help, "acquire_bus") {
		t.Fatalf("did not expect acquire_bus for prefix wr, got:\n%s", help)
	}
}

func TestContextHelpArgByPosition(t *testing.T) {
	r := DefaultCommands()
	cases := []struct {
		line string
		want string
	}{
		{"write_flash ", "<file>"},
		{"write_flash fir", "<file>"},
		{"write_flash firmware.bin ", "<address>"},
		{"read_flash dump.bin ", "[address]"},
		{"read_flash dump.bin 0x0 ", "[size]"},
		{"acquire_bus ", "no more arguments"},
	}
	for _, c := range cases {
		got := contextHelp(r, c.line)
		if !strings.Contains(got, c.want) {
			t.Errorf("contextHelp(%q) = %q, want substring %q", c.line, got, c.want)
		}
	}
}

func TestOptionalArgDefaults(t *testing.T) {
	cmd, _ := DefaultCommands().Lookup("read_flash")
	if cmd.requiredArgs() != 1 {
		t.Fatalf("requiredArgs = %d, want 1", cmd.requiredArgs())
	}
	if cmd.maxArgs() != 3 {
		t.Fatalf("maxArgs = %d, want 3", cmd.maxArgs())
	}
	filled := cmd.fillDefaults([]string{"dump.bin"})
	want := []string{"dump.bin", "0", "32768"}
	if !slices.Equal(filled, want) {
		t.Fatalf("fillDefaults = %v, want %v", filled, want)
	}
}

func TestRemoveRuneBefore(t *testing.T) {
	line := []rune("write_flash ?")
	stripped, pos := removeRuneBefore(line, len(line))
	if string(stripped) != "write_flash " {
		t.Fatalf("stripped = %q, want %q", string(stripped), "write_flash ")
	}
	if pos != len("write_flash ") {
		t.Fatalf("pos = %d, want %d", pos, len("write_flash "))
	}
}

func TestFileCandidates(t *testing.T) {
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "firmware.bin"), nil, 0644); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(filepath.Join(dir, "subdir"), 0755); err != nil {
		t.Fatal(err)
	}
	prefix := filepath.Join(dir, "f")
	candidates := fileCandidates("write_flash " + prefix)
	if !slices.Contains(candidates, filepath.Join(dir, "firmware.bin")) {
		t.Fatalf("expected firmware.bin candidate, got %v", candidates)
	}
	if !slices.Contains(candidates, filepath.Join(dir, "subdir")+string(filepath.Separator)) {
		t.Fatalf("expected subdir/ candidate, got %v", candidates)
	}
}
