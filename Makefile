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

all: create
	$(MAKE) -C source all
	cp $(SRCDIR)/*lib$(LIBNAME).* lib/
	for f in $(SUBDIRS); do $(MAKE) -C $$f all; done

clean:
	$(MAKE) -C source clean
	for f in $(SUBDIRS); do $(MAKE) -C $$f clean; done
	rm -f lib/*

clone: create
	cp $(SRCDIR)/*.h include/myhtml
	cp $(SRCDIR)/utils/*.h include/myhtml/utils
	find include/myhtml -type f -name '*.h' -exec sed -i .old -e 's/^#include "/#include "myhtml\//g' {} +
	find include/myhtml/utils -type f -name '*.c' -exec sed -i .old -e 's/^#include "/#include "myhtml\/utils\//g' {} +
	rm -rf include/myhtml/*.old include/myhtml/utils/*.old

create:
	mkdir -p bin lib include/myhtml/utils

.PHONY:all clean