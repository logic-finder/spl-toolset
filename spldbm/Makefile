## Variables ##
TARGET := spldbm
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

BUILD_DB := db

## Targets ##
.PHONY: all $(BUILD_DB) clean install uninstall help

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
$(BUILD_DB):
	$(TARGET) --make=../dat/name,../dat/adj,../dat/noun,../dat/comp

clean:
	rm -f $(TARGET) *.o *.d

# TODO:
#  - copy word DB under /usr/local/share/splt
#  - copy the executable under /usr/local/bin
#  - copy the manpage /usr/local/share/man/man1
install: $(TARGET) $(BUILD_DB)

uninstall:

# TODO: write help
help:
