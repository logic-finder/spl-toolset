## Variables ##
TARGETS = $(SPLC) $(SPLDBM) $(SPLRT)
SHELL := /bin/sh

SPLC := splc
SPLDBM := spldbm
SPLRT := splrt

CLEAN_TARGETS = $(addprefix clean-,$(TARGETS))
INSTALL_TARGETS = $(addprefix install-,$(TARGETS))
UNINSTALL_TARGETS = $(addprefix uninstall-,$(TARGETS))

## Targets ##
.PHONY: all $(TARGETS)
.PHONY: clean $(CLEAN_TARGETS)
.PHONY: install $(INSTALL_TARGETS)
.PHONY: uninstall $(UNINSTALL_TARGETS)
.PHONY: help

# Default Goal #
all: $(TARGETS)

$(TARGETS):
	$(MAKE) --directory=$@

# Tasks #
clean: $(CLEAN_TARGETS)

$(CLEAN_TARGETS):
	$(MAKE) --directory=$(patsubst clean-%,%,$@) clean

# TODO: copy the project manpage under /usr/local/share/man/man7
install: $(INSTALL_TARGETS)

$(INSTALL_TARGETS): install-%: %
	$(MAKE) --directory=$(patsubst install-%,%,$@) install

uninstall: $(UNINSTALL_TARGETS)

$(UNINSTALL_TARGETS):
	$(MAKE) --directory=$(patsubst uninstall-%,%,$@) uninstall

# TODO: write help
help:
