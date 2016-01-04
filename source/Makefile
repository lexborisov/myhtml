TARGET := source
SRCDIR := source

CC ?= gcc

LIBPOSTFIX := .so
LIBNAME    := myhtml
LIBSTATIC_PREFIX  := static_

ifeq ($(OS),Windows_NT)
else
	UNAM := $(shell uname -s)
	ifeq ($(UNAM),Darwin)
		LIBPOSTFIX := .dylib
		CFLAGS ?= -Wall -Werror -O2 --std=c99 -pthread -I.
	else
		CFLAGS ?= -Wall -Werror -O2 --std=c99 -pthread -D_POSIX_C_SOURCE=199309L -I.
	endif
endif

SRCS := $(wildcard *.c)
SRCS += $(wildcard utils/*.c)
HDRS := $(wildcard *.h)
HDRS += $(wildcard utils/*.h)
OBJS := $(patsubst %.c, %.o, $(SRCS))

all: shared static

shared: $(OBJS) $(HDRS)
	$(CC) -shared $(LDFLAGS) $(OBJS) -o lib$(LIBNAME)$(LIBPOSTFIX)

static: shared
	$(AR) crus $(LIBSTATIC_PREFIX)lib$(LIBNAME).a $(OBJS)

clean:
	rm -rf *.o
	rm -rf utils/*.o
	rm -rf *lib$(LIBNAME)*

.PHONY: all clean
