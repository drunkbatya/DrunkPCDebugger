$(BUILD_DIR):
	@mkdir -p $@

$(PROTO_OUT_DIR):
	@mkdir -p $@

$(NANOPB_GENERATED_C): $(PROTO_DIR)/$(PROTO_FILE) $(MAKE_FILES) | $(PROTO_OUT_DIR)
	@echo "\tPROTO\t" $<
	@$(PROTO_GENERATOR) -I$(PROTO_DIR) --output-dir=$(PROTO_OUT_DIR) $(PROTO_FILE)

$(NANOPB_GENERATED_H): $(NANOPB_GENERATED_C)
	@:

$(BUILD_DIR)/%.o: %.c $(MAKE_FILES) | $(BUILD_DIR)
	@echo "\tCC\t" $<
	@mkdir -p $(@D)
	@$(CC) -c $(CFLAGS) $< -o $@

$(C_OBJECTS): $(GENERATED_HEADERS)

$(BUILD_DIR)/%.o: %.s $(MAKE_FILES) | $(BUILD_DIR)
	@echo "\tASM\t" $<
	@mkdir -p $(@D)
	@$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(MAKE_FILES) | $(BUILD_DIR)
	@echo "\tLD\t" $@
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "\tHEX\t" $@
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "\tBIN\t" $@
	@$(BIN) $< $@

-include $(OBJECTS:.o=.d)
