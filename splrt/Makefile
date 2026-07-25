## Variables ##
NAME := splrt
TARGET = lib$(NAME).so
LIBDIR := ../lib
SHELL := /bin/sh

CC := gcc
CFLAGS := -O -Wall -W -pedantic
CFLAGS += -g
CFLAGS += -MMD -MP
CFLAGS += -fpic
CPPFLAGS = -I $(LIBDIR)

SOURCES = $(wildcard *.c)
LIBSRCS = $(wildcard $(LIBDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
LIBOBJS = $(notdir $(LIBSRCS:.c=.o))
DEPENDS = $(OBJECTS:.o=.d) $(LIBOBJS:.o=.d)

## Targets ##
.PHONY: all $(TARGET)
.PHONY: clean build install uninstall help

# Default Goal #
all: $(TARGET)

# Build #
$(TARGET): $(OBJECTS) $(LIBOBJS)
	$(CC) $^ $(CFLAGS) -shared -o $@

$(OBJECTS): %.o: %.c
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

$(LIBOBJS): %.o: $(LIBDIR)/%.c
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

# Dependency #
-include $(DEPENDS)

# Tasks #
clean:
	rm -f $(TARGET) *.o *.d

# Note: all prerequisites must be installed first
#  - words.spldb
#  - libsplrt.so
# Note: need to specify SPLT_OUTPUT
build:
	$(CC) $(CPPFLAGS) $(CFLAGS) -l$(NAME) -lm \
		-o $(basename $(SPLT_OUTPUT)) $(SPLT_OUTPUT)

# TODO:
#  - copy libsplrt.so under ???
#  - copy the executable under /usr/local/bin
#  - copy the manpage /usr/local/share/man/man1
install: $(TARGET)

uninstall:

# TODO: write help
help:
