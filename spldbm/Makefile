srcdir := .
objdir := ../.object
depdir := ../.depend
common_source_dir := ..
common_header_dir := ..
common_object_dir := $(objdir)/common

target := spldbm
sources := $(wildcard $(srcdir)/*.c)
common_sources := $(wildcard $(common_source_dir)/*.c)
objects := $(sources:$(srcdir)/%.c=$(objdir)/$(target)/%.o)
common_objects := $(common_sources:$(common_source_dir)/%.c=$(common_object_dir)/%.o)
depends := $(sources:$(srcdir)/%.c=$(depdir)/$(target)/%.d)
common_depends := $(common_sources:$(common_source_dir)/%.c=$(depdir)/common/%.d)

SHELL := /bin/sh
CC := gcc
CFLAGS := -g -Wall -W -pedantic
CPPFLAGS := -I $(common_header_dir)

.SUFFIXES:
.SUFFIXES: .c .o .h

.PHONY: all
all: $(target)

$(target): $(objects) $(common_objects)
	$(CC) $^ $(CFLAGS) -o $(target)

$(depdir)/$(target)/%.d: $(srcdir)/%.c
	@set -e; rm -f $@; \
	 $(CC) $< $(CPPFLAGS) -MM > $@.$$$$; \
	 printf "%s%s" "$(objdir)/$(target)/" "$$(cat $@.$$$$)" > $@; \
	 rm -f $@.$$$$

$(depdir)/common/%.d: $(common_source_dir)/%.c
	@set -e; rm -f $@; \
	 $(CC) $< $(CPPFLAGS) -MM > $@.$$$$; \
	 printf "%s%s" "$(objdir)/common/" "$$(cat $@.$$$$)" > $@; \
	 rm -f $@.$$$$

include $(depends) $(common_depends)

$(objdir)/$(target)/%.o:
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

$(objdir)/common/%.o:
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

.PHONY: clean
clean:
	rm -rf $(target) $(depdir)/**/*.d $(objdir)/*.o
