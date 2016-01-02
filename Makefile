TARGET := myhtml
SRCDIR := myhtml

CC ?= gcc
CFLAGS ?= -Wall -std=c99 -I. -Iutils

SRCS := $(wildcard $(SRCDIR)/*.c)
HDRS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst %.c,%.o,$(SRCS))

SUBDIRS := myhtml 

all: 
	for f in $(SUBDIRS); do $(MAKE) -C $$f all; done

clean: 
	for f in $(SUBDIRS); do $(MAKE) -C $$f clean; done

.PHONY:all clean