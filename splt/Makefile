## Variables ##
TARGET := splt
LIBDIR := ../lib
SHELL := /bin/sh

CC := gcc
CFLAGS := -O -Wall -W -pedantic
CFLAGS += -g
CFLAGS += -MMD -MP
CPPFLAGS = -I $(LIBDIR)

SOURCES = $(wildcard *.c)
LIBSRCS = $(wildcard $(LIBDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
LIBOBJS = $(notdir $(LIBSRCS:.c=.o))
DEPENDS = $(OBJECTS:.o=.d) $(LIBOBJS:.o=.d)

## Targets ##
.PHONY: all $(TARGET)
.PHONY: clean install uninstall run help

# Default Goal #
all: $(TARGET)

# Build #
$(TARGET): $(OBJECTS) $(LIBOBJS)
	$(CC) $^ $(CFLAGS) -o $@

$(OBJECTS): %.o: %.c
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

$(LIBOBJS): %.o: $(LIBDIR)/%.c
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

# Dependency #
-include $(DEPENDS)

# Tasks #
clean:
	rm -f $(TARGET) *.o *.d

# TODO:
#  - copy the executable under /usr/local/bin
#  - copy the manpage /usr/local/share/man/man1
install: $(TARGET)

uninstall:

run:
	$(TARGET) -- ../dat/hello.spl

# TODO: write help
help:
