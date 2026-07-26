all: $(BUILD_DIR)/$(TARGET)

.PHONY: clean
clean:
	-rm -fR $(BUILD_DIR)
	-rm -fR $(PROTO_OUT_DIR)
