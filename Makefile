TARGET := source/myhtml
SRCDIR := source/myhtml

CC ?= gcc

SRCS := $(wildcard $(SRCDIR)/*.c)
HDRS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst %.c,%.o,$(SRCS))

SUBDIRS := examples
LIBNAME := myhtml

all: create
	$(MAKE) -C $(SRCDIR) $@
	cp $(SRCDIR)/*lib$(LIBNAME)*.* lib/
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