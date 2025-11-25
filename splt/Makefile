srcdir := .
common_source_dir := ..
objdir := ../.object
depdir := ../.depend
common_header_dir := ../

target := splt
sources := $(wildcard $(srcdir)/*.c)
common_sources := $(wildcard $(common_source_dir)/*.c)
objects := $(sources:$(srcdir)/%.c=$(objdir)/%.o)
common_objects := $(common_sources:$(common_source_dir)/%.c=$(objdir)/%.o)
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

# canned recipe
define make-depend = 
@set -e; rm -f $@; \
 $(CC) $< $(CPPFLAGS) -MM > $@.$$$$; \
 printf "%s%s" "$(objdir)/" "$$(cat $@.$$$$)" > $@; \
 rm -f $@.$$$$
endef

$(depdir)/$(target)/%.d: $(srcdir)/%.c
	$(make-depend)

$(depdir)/common/%.d: $(common_source_dir)/%.c
	$(make-depend)

include $(depends) $(common_depends)

$(objdir)/%.o:
	$(CC) $< $(CPPFLAGS) $(CFLAGS) -c -o $@

.PHONY: clean
clean:
	rm -rf $(target) $(depdir)/**/*.d $(objdir)/*.o
