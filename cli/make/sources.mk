GO_FILES := $(shell find . \
    -path './.*' -prune -o \
    -path './$(PROTO_OUT_DIR)/*' -prune -o \
    -type f -name '*.go' -print)
