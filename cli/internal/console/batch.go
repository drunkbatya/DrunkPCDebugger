package console

func RunBatch(r *Registry, s *Session, lines []string) error {
	for _, line := range lines {
		s.Printf("> %s\n", line)
		if err := r.DispatchBatch(s, line); err != nil {
			return err
		}
	}
	return nil
}
