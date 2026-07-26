$(BUILD_DIR):
	@mkdir -p $@

$(PROTO_OUT_DIR):
	@mkdir -p $@

$(PROTO_GENERATED): $(PROTO_DIR)/$(PROTO_FILE) $(MAKE_FILES) $(PROTO_DIR)/$(PROTO_OPTIONS) | $(PROTO_OUT_DIR)
	@echo "\tPROTO\t" $<
	@$(PROTO_GENERATOR) "-I$(PROTO_DIR)" "--go_out=$(PROTO_OUT_DIR)" \
		--go_opt=paths=source_relative \
		--go_opt=M$(PROTO_FILE)=$(TARGET)/$(PROTO_OUT_DIR) \
		"$(PROTO_FILE)"

$(BUILD_DIR)/$(TARGET): $(MOD_FILES) $(GO_FILES) $(PROTO_GENERATED) $(MAKE_FILES) | $(BUILD_DIR)
	@echo "\tGO\t$(TARGET)"
	@$(GO) build \
		-ldflags "$(LDFLAGS)" \
		-o $(BUILD_DIR)/$(TARGET) \
		$(CMD_PACKAGE)
