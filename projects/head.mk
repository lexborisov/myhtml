CC = gcc

PATH_MK   := $(abspath $(firstword $(MAKEFILE_LIST)))
PATH_CDIR := $(dir $(PATH_MK))..
PATH_SRC   = $(PATH_CDIR)/myhtml

CINC    = -I$(PATH_CDIR)/myhtml
LIBINC  = -pthread
CFLAGS  =  -Wall -fPIC -O2 -std=gnu99 $(CINC) $(LIBINC)
LDFLAGS = -shared
