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

LVGL_DIR := $(TOP_DIR)
LVGL_DIR_NAME := lvgl
LVGL_GIT_REF := release/v8.3
LV_DRIVERS_GIT_REF := release/v8.3

CC := $(CROSS_COMPILE)gcc
CFLAGS += -Wall -Wshadow -Wundef -Wmaybe-uninitialized -O3 -g0 \
	  -I$(TOP_DIR) \
	  $(CFLAGS_USER)
LDFLAGS ?= -lrt -lpthread -lgpiod $(LDFLAGS_USER)

-include $(LVGL_DIR)/$(LVGL_DIR_NAME)/lvgl.mk
-include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

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

## MAINOBJ -> OBJFILES
.PHONY: all
all: $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): lvgl-git-check $(OBJS)
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)
	@echo "CC -o $@"

$(BUILD_DIR)/%.o: %.c | lvgl-git-check
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $(subst $(CURDIR)/,,$<)"

clean:
	rm -rf $(BUILD_DIR)

lvgl-git-%:
	@if ! [ -d lvgl ]; then \
		git clone https://github.com/lvgl/lvgl; \
		if ! [ -d lvgl ]; then \
			printf "***** LVGL GIT CLONE FAILED *****\n"; \
			exit 2; \
		fi; \
		(cd lvgl && git checkout $(LVGL_GIT_REF)); \
	else \
		if ! [ "check" = "$(*F)" ]; then \
			(cd lvgl && git $(*F)); \
		fi; \
	fi
	@if ! [ -d lv_drivers ]; then \
		git clone https://github.com/lvgl/lv_drivers; \
		if ! [ -d lvgl ]; then \
			printf "***** LVGL DRIVERS GIT CLONE FAILED *****\n"; \
			exit 2; \
		fi; \
		(cd lvgl && git checkout $(LV_DRIVERS_GIT_REF)); \
	else \
		if ! [ "check" = "$(*F)" ]; then \
			(cd lv_drivers && git $(*F)); \
		fi; \
	fi

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
