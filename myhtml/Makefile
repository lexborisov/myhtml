TARGET := myhtml
SRCDIR := myhtml

CC := gcc

ifeq ($(OS),Windows_NT)
else
    UNAM := $(shell uname -s)
    ifeq ($(UNAM),Darwin)
        CFLAGS ?= -Wall -Werror -O2 --std=c99 -I. -Iutils
    else
        CFLAGS ?= -Wall -Werror --std=c99 -D_POSIX_C_SOURCE=199309L -I. -Iutils
    endif
endif

SRCS := $(wildcard *.c)
SRCS += $(wildcard utils/*.c)
HDRS := $(wildcard *.h)
HDRS += $(wildcard utils/*.h)
OBJS := $(patsubst %.c, %.o, $(SRCS))
LIBS := -lpthread

all: $(TARGET)

$(TARGET): $(OBJS) $(HDRS) Makefile
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

clean:
	rm -rf *.o 

.PHONY: all clean
