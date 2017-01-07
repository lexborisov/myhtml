TARGET := source/myhtml
SRCDIR := source/myhtml

CC ?= gcc

SRCS := $(wildcard $(SRCDIR)/*.c)
HDRS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst %.c,%.o,$(SRCS))

SUBDIRS := examples
LIBNAME := myhtml

all: lib examples

lib: create
	$(MAKE) -C $(SRCDIR)
	cp $(SRCDIR)/*lib$(LIBNAME)*.* lib/

examples: lib
	for f in $(SUBDIRS); do $(MAKE) -C $$f all; done

clean:
	$(MAKE) -C $(SRCDIR) clean
	$(MAKE) -C test clean
	for f in $(SUBDIRS); do $(MAKE) -C $$f clean; done
	rm -rf lib/*

clone: create
	find include -name "*.h" -exec rm -f {} \;
	cp $(SRCDIR)/*.h include/myhtml
	cp $(SRCDIR)/utils/*.h include/myhtml/utils
	find include -name "*.h" -exec sed -i '.bak' -E 's/^[ \t]*#[ \t]*include[ \t]*"([^"]+)"/#include <\1>/g' {} \;
	find include -name "*.h.bak" -exec rm -f {} \;

create:
	mkdir -p bin lib include/myhtml/utils

build_test:
	$(MAKE) -C test

.PHONY: all lib examples clean
