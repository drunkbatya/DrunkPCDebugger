TARGET ?= DrunkPCDebugger
HW_TARGET ?= d1
HW_TARGET_ID ?= $(subst d,,$(HW_TARGET))
DEBUG ?= 1
OPT ?= -Og
BUILD_DIR ?= build
JOBS ?= $(shell nproc 2>/dev/null || echo 1)

ifeq ($(filter -j% --jobs%,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(JOBS)
endif

OPENOCD_CFG ?= -f interface/stlink.cfg -f target/stm32h7x.cfg

MAKE_FILES := $(shell find make -type f -name '*.mk')

PROTO_DIR ?= ../protobuf
PROTO_FILE ?= debugger.proto
PROTO_OUT_DIR ?= generated
PROTO_GENERATOR ?= ./src/lib/nanopb/generator/nanopb_generator.py
PROTO_BASENAME := $(basename $(notdir $(PROTO_FILE)))
NANOPB_GENERATED_C := $(PROTO_OUT_DIR)/$(PROTO_BASENAME).pb.c
NANOPB_GENERATED_H := $(PROTO_OUT_DIR)/$(PROTO_BASENAME).pb.h
