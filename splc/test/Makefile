## Variables ##
SRCDIR := ../
LIBDIR := ../../lib
SHELL := /bin/sh

CC := gcc
CFLAGS := -Wall -W -pedantic
CFLAGS += -g --std=c2x
# CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined
CFLAGS += -MMD -MP
CPPFLAGS = -I $(SRCDIR)  -I $(LIBDIR)

UNITS = $(wildcard *.c)
EXECS = $(basename $(UNITS))

SOURCES = $(addprefix $(SRCDIR),config.c dbhandler.c msg.c)
LIBSRCS = $(wildcard $(LIBDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
LIBOBJS = $(notdir $(LIBSRCS:.c=.o))
DEPENDS = $(OBJECTS:.o=.d) $(LIBOBJS:.o=.d)

## Targets ##
.PHONY: all clean run

# Default Goal #
all: $(EXECS)

# Build #
$(EXECS): %: %.o $(OBJECTS) $(LIBOBJS)
	$(CC) $^ $(CPPFLAGS) $(CFLAGS) -o $@

$(OBJECTS): %.o: %.c
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

$(LIBOBJS): %.o: $(LIBDIR)/%.c
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

# Dependency #
-include $(DEPENDS)

# Tasks #
clean:
	rm -f $(EXECS) *.o *.d

run: all
	@for prog in $(EXECS); do ./$$prog; done
