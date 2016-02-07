TARGET := source/myhtml
SRCDIR := source/myhtml

CC ?= gcc
CFLAGS ?= -Wall -std=c99 -I.. -pthread

SRCS := $(wildcard $(SRCDIR)/*.c)
HDRS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst %.c,%.o,$(SRCS))

CFLAGS  += -pthread
SUBDIRS := examples
LIBNAME := myhtml

all: create
	$(MAKE) -C $(SRCDIR) all
	cp $(SRCDIR)/*lib$(LIBNAME).* lib/
	for f in $(SUBDIRS); do $(MAKE) -C $$f all; done

clean:
	$(MAKE) -C $(SRCDIR) clean
	for f in $(SUBDIRS); do $(MAKE) -C $$f clean; done
	rm -rf lib/*

clone: create
	cp $(SRCDIR)/*.h include/myhtml
	cp $(SRCDIR)/utils/*.h include/myhtml/utils

create:
	mkdir -p bin lib include/myhtml/utils

.PHONY:all clean