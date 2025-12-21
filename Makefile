.PHONY: all
all:
	cd splt && $(MAKE)
	cd spldbm && $(MAKE)

.PHONY: clean
clean:
	rm -f splt/splt spldbm/spldbm
	rm -f .depend/**/*.d .object/*.o
