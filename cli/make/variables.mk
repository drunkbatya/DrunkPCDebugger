TARGET ?= drunkpc-debugger
BUILD_DIR ?= build
CMD_PACKAGE ?= cmd/$(TARGET)/main.go
JOBS ?= $(shell nproc 2>/dev/null || echo 1)

ifeq ($(filter -j% --jobs%,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(JOBS)
endif

MAKE_FILES := $(shell find make -type f -name '*.mk')

MOD_FILES := go.mod go.sum

PROTO_PACKAGE ?= debugger
PROTO_DIR ?= ../protobuf
PROTO_OUT_DIR ?= generated
PROTO_FILE ?= $(PROTO_PACKAGE).proto
PROTO_OPTIONS ?= $(PROTO_PACKAGE).options
PROTO_BASENAME := $(basename $(notdir $(PROTO_FILE)))
PROTO_GENERATED := $(PROTO_OUT_DIR)/$(PROTO_BASENAME).pb.go
