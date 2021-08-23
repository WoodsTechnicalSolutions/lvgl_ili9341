#
# Makefile
#

-include user.mk

CC ?= gcc
LVGL_DIR ?= $(shell pwd)
LVGL_DIR_NAME ?= lvgl
CFLAGS ?= -Wall -Wshadow -Wundef -Wmaybe-uninitialized -O3 -g0 -I$(LVGL_DIR)/ $(LDFLAGS_USER)
LDFLAGS ?= -lrt -lpthread -lgpiod $(LDFLAGS_USER)
BIN = ili9341

# Collect the files to compile

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
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(BIN)
$(BIN): $(OBJS)
	@$(CC) -o $(BIN) $(OBJS) $(LDFLAGS)
	@echo "CC -o $@"

clean: 
	rm -f $(BIN) $(OBJS)
