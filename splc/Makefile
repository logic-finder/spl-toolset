## Variables ##
TARGET := splc
LIBDIR := ../lib
SHELL := /bin/sh

CC := gcc
CFLAGS := -Wall -W -pedantic
CFLAGS += -O0 -g
CFLAGS += -MMD -MP
CPPFLAGS = -I $(LIBDIR)

SOURCES = $(wildcard *.c)
LIBSRCS = $(wildcard $(LIBDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
LIBOBJS = $(notdir $(LIBSRCS:.c=.o))
DEPENDS = $(OBJECTS:.o=.d) $(LIBOBJS:.o=.d)

## Targets ##
.PHONY: all clean install uninstall help run

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
	man ./splc.1 > splc.1.txt
	install -d /usr/local/share/splc
	install -m 644 splc.1.txt /usr/local/share/splc

uninstall:

# TODO: write help
help:

run:
	$(TARGET) -- ../dat/hello.spl
