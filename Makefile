#
# Makefile
#
CC ?= /src/etinker/toolchain/arm-cortexa9-linux-gnueabihf/bin/arm-cortexa9-linux-gnueabihf-gcc --sysroot=/src/etinker/rootfs/build/zynq-xlnx/arm-cortexa9-linux-gnueabihf/staging
LVGL_DIR ?= $(shell pwd)
LVGL_DIR_NAME ?= lvgl
CFLAGS ?= -Wall -Wshadow -Wundef -Wmaybe-uninitialized -O3 -g0 -I$(LVGL_DIR)/
LDFLAGS ?= -lrt -lgpiod
BIN = ili9341


#Collect the files to compile
MAINSRC = ./main.c ./io.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS) $(MAINOBJ)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(BIN)
$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN) $(OBJS)

