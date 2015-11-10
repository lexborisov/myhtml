include head.mk

SOURCES = $(PATH_SRC)/main.c $(PATH_SRC)/myhtml.c $(PATH_SRC)/mcobject.c $(PATH_SRC)/mctree.c \
$(PATH_SRC)/myhtml_parse_doctype.c $(PATH_SRC)/myhtml_parse_script.c $(PATH_SRC)/myhtml_parse.c \
$(PATH_SRC)/myhtml_queue.c $(PATH_SRC)/myhtml_rules.c $(PATH_SRC)/myhtml_string.c \
$(PATH_SRC)/myhtml_thread.c $(PATH_SRC)/myhtml_token.c $(PATH_SRC)/myhtml_tree.c \
$(PATH_SRC)/mytags_init.c $(PATH_SRC)/mytags.c

OBJECTS = $(SOURCES:.c=.o)

LIBEDIR = $(PATH_CDIR)../lib

EXENAME = myhtml
EXEEDIR = $(PATH_CDIR)/bin

all: execute

execute: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEEDIR)/$(EXENAME) $(OBJECTS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(PATH_SRC)/*.o

