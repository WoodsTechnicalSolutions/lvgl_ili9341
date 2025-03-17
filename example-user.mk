CROSS_PREFIX := <arch>-<vendor>-<os>-<eabi>
CROSS_COMPILE := $(CROSS_PREFIX)-
TOOLCHAIN_DIR := /path/to/toolchain/$(CROSS_PREFIX)
STAGING_DIR := /path/to/staging

export PATH := $(TOOLCHAIN_DIR)/bin:$(PATH)

CC := $(CROSS_COMPILE)gcc
CFLAGS_USER := -isysroot $(shell $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)gcc -print-sysroot) \
	       -I$(STAGING_DIR)/usr/include
LDFLAGS_USER := --sysroot=$(shell $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)gcc -print-sysroot) \
	       -L$(STAGING_DIR)/usr/lib
