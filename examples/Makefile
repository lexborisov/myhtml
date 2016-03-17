TARGET := examples
SRCDIR := examples

CC ?= gcc
LIBS := ../lib/static_libmyhtml.a
LDLIBS := $(LIBS)

MyHTML_OPTIMIZATION_LEVEL ?= -O2

CFLAGS ?= -Wall -Werror
CFLAGS += $(MyHTML_OPTIMIZATION_LEVEL) -fPIC --std=c99 -I../include

MyHTML_BUILD_WITHOUT_THREADS ?= NO
ifeq ($(MyHTML_BUILD_WITHOUT_THREADS),YES)
    $(info Build Examples without POSIX Threads)
else
    $(info Build Examples with POSIX Threads)
	CFLAGS += -pthread
endif

ifeq ($(OS),Windows_NT)
else
	UNAM := $(shell uname -s)
	ifeq ($(UNAM),Darwin)
	else
		CFLAGS += -D_POSIX_C_SOURCE=199309L
	endif
endif

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, %, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	cp -r $(OBJS) ../bin/

clean:
	rm -f `ls | grep -v '\\.c\|Makefile$\'`
	rm -fr ../bin/*
.PHONY: all clean
