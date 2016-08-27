TARGET := text
SRCDIR := text

CC ?= gcc
LIBS := ../lib/libmyhtml_static.a
LDLIBS := $(LIBS)

MyHTML_OPTIMIZATION_LEVEL ?= -O2

CFLAGS ?= -Wall -Werror
CFLAGS += $(MyHTML_OPTIMIZATION_LEVEL) -fPIC --std=c99 -I../include

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
	rm -rf ../bin/*

.PHONY: all clean
