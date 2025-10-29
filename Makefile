#
# Makefile
#
# Copyright (C) 2020-2025, Derald D. Woods <woods.technical@gmail.com>
#
# This file is made available under the terms of the GNU General Public
# License version 3.
#

-include user.mk

TOP_DIR := $(shell pwd)

export LVGL_GIT_REF := release/v9.4

READY := $(shell $(TOP_DIR)/scripts/git check)

CC := $(CROSS_COMPILE)gcc
CFLAGS += -Wall -Wshadow -Wundef -Wmaybe-uninitialized -O3 -g0 \
	  -I$(TOP_DIR) \
	  $(CFLAGS_USER)
LDFLAGS ?= -z noexecstack -lrt -lpthread -lgpiod -ldrm $(LDFLAGS_USER)

-include lvgl.mk

BIN = ili9341

# Collect the files to compile

OBJEXT ?= .o

BUILD_DIR := build

lvcsrc := $(subst $(CURDIR)/,,$(CSRCS))
CSRCS := $(lvcsrc)
lvasrc := $(subst $(CURDIR)/,,$(ASRCS))
ASRCS := $(lvasrc)
AOBJS := $(ASRCS:%.S=$(BUILD_DIR)/%$(OBJEXT))
COBJS := $(CSRCS:%.c=$(BUILD_DIR)/%$(OBJEXT))

MAINSRC := $(wildcard *.c)
MAINOBJ := $(MAINSRC:%.c=$(BUILD_DIR)/%$(OBJEXT))

SRCS := $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS := $(AOBJS) $(COBJS) $(MAINOBJ)

.PHONY: all
all: $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): git-check $(OBJS)
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)
	@echo "CC -o $@"

$(BUILD_DIR)/%.o: %.c | git-check
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $(subst $(CURDIR)/,,$<)"

$(BUILD_DIR)/%.o: %.S | git-check
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $(subst $(CURDIR)/,,$<)"

clean:
	rm -rf $(BUILD_DIR)
	rm -f .git-ready

.PHONY: git
git: .git-ready

.git-ready:
	@$(TOP_DIR)/scripts/git check
	@touch $@

git-%:
	@$(TOP_DIR)/scripts/git $(*F)

.PHONY: info
info:
	@printf "BIN = $(BIN)\n"
	@printf "CROSS_COMPILE = $(CROSS_COMPILE)\n"
	@printf "CC = $(CC)\n"
	@printf "CC = $(CC)\n"
	@printf "CFLAGS  = $(CFLAGS)\n"
	@printf "LDFLAGS = $(LDFLAGS)\n"
	@printf "SRCS = $(SRCS)\n"
	@printf "OBJS = $(OBJS)\n"
