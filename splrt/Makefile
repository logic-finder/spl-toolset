## Variables ##
NAME := splrt
TARGET = lib$(NAME).so
LIBDIR := ../lib
SHELL := /bin/sh

CC := gcc
CFLAGS := -Wall -W -pedantic
CFLAGS += -O0 -g
CFLAGS += -MMD -MP
CFLAGS += -fpic
CPPFLAGS = -I $(LIBDIR) -I .

SOURCES = $(wildcard *.c)
LIBSRCS = $(wildcard $(LIBDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
LIBOBJS = $(notdir $(LIBSRCS:.c=.o))
DEPENDS = $(OBJECTS:.o=.d) $(LIBOBJS:.o=.d)

INSTALL := install
INSTALL_DATA = $(INSTALL) -m 644
INSTALL_LIB = $(INSTALL) -m 755
INSTALL_DIR = $(INSTALL) -d

PREFIX := /usr/local
SODIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include/$(NAME)
MANDIR = $(PREFIX)/share/man/man3

## Targets ##
.PHONY: all build clean install uninstall help

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
# Note: all prerequisites must be installed first
#  - words.spldb
#  - libsplrt.so
# Note: need to specify SPLC_OUTPUT
build: install
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SPLC_OUTPUT) \
		-l$(NAME) -lm -o $(basename $(SPLC_OUTPUT))

clean:
	rm -f $(TARGET) *.o *.d

install: $(TARGET)
	$(INSTALL_DIR) $(SODIR)
	$(INSTALL_DIR) $(INCDIR)
	$(INSTALL_DIR) $(MANDIR)

	$(INSTALL_LIB) $(TARGET) $(SODIR)
	$(INSTALL_DATA) $(NAME).h global.h stage.adt.h $(INCDIR)
	$(INSTALL_DATA) $(TARGET:.so=.3) $(MANDIR)

uninstall:
	rm -f $(MANDIR)/$(TARGET:.so=.3)
	rm -f $(INCDIR)/$(NAME).h
	rm -f $(SODIR)/$(TARGET)

help:
	@echo "   make - builds the library"
	@echo "   make build - links the library with a transpiled c program"
	@echo "   make clean - removes every build artifact"
	@echo "   make install - installs the library, header, and manual page under standard directories"
	@echo "   make uninstall - deletes the installed files"
	@echo "   make help - prints this manual"
