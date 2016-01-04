TARGET := source
SRCDIR := source

CC ?= gcc
CFLAGS ?= -Wall -std=c99 -I. -pthread

SRCS := $(wildcard $(SRCDIR)/*.c)
HDRS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst %.c,%.o,$(SRCS))

CFLAGS  += -pthread
SUBDIRS := examples
LIBNAME := myhtml

all:
	mkdir -p bin lib
	$(MAKE) -C source all
	cp $(SRCDIR)/*lib$(LIBNAME).* lib/
	for f in $(SUBDIRS); do $(MAKE) -C $$f all; done

clean:
	$(MAKE) -C source clean
	for f in $(SUBDIRS); do $(MAKE) -C $$f clean; done
	rm -f lib/*

.PHONY:all clean