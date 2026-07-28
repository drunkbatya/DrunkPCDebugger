.PHONY: all
all: $(BUILD_DIR)/$(TARGET)

.PHONY: build
build: $(BUILD_DIR)/$(TARGET)

.PHONY: format
format:
	@$(GO) fmt ./...

.PHONY: lint
lint:
	@UNFORMATTED=$$($(GOFMT) -l $(GO_FILES)); \
	if [ -n "$$UNFORMATTED" ]; then \
		echo "gofmt needed on:"; echo "$$UNFORMATTED"; exit 1; \
	fi

.PHONY: vet
vet: $(PROTO_GENERATED)
	$(GO) vet ./...

.PHONY: test
test: $(PROTO_GENERATED)
	$(GO) test ./...

.PHONY: clean
clean:
	-rm -fR $(BUILD_DIR)
	-rm -fR $(PROTO_OUT_DIR)
