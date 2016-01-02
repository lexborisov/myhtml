TARGET := myhtml
SRCDIR := myhtml

CC := gcc
CFLAGS ?= -Wall -Werror --std=c99 -D_POSIX_C_SOURCE=199309L -I. -Iutils


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
