SRCS 	:= $(shell find src -name "*.c")
HEADERS := $(shell find include -name "*.h")
OBJS 	:= $(SRCS:src/%.c=objects/%.o)
IR   	:= $(wildcard src/*.ir)
CC   	:= gcc
CFLAGS 	:= $(shell pkg-config --cflags bdw-gc) -I./include/ -O2 -Wall -Wextra -Wno-unused-parameter
LDFLAGS := $(shell pkg-config --libs bdw-gc)
OBJECTS := objects
# Can add -g flag to .c and .ir rules to get debug symbols

ATOM_H := include/atoms.h

all: atomgen
	@$(MAKE) --no-print-directory iridium

iridium: $(OBJECTS) $(OBJECTS)/iridium.o

atomgen: $(SRCS) $(HEADERS)
	@echo GEN $(ATOM_H)
	../scripts/atomgen include/atoms.txt $(ATOM_H) src/atoms.c $^

$(OBJECTS)/iridium.o: $(OBJS) $(OBJECTS)/core.o $(OBJECTS)/atoms.o
	@echo LD $@
	ld -r -o $@ $(OBJS) $(OBJECTS)/core.o

$(OBJS): $(OBJECTS)/%.o: src/%.c $(ATOM_H)
	@echo CC $@
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJECTS):
	mkdir -p $(OBJECTS)
	mkdir -p $(OBJECTS)/shared

$(OBJECTS)/core.o: $(IR)
	@echo IR $@
	ircc -o $@ -c src/core.ir --main=IR_CORE_INIT

clean:
	rm -rf $(OBJECTS)

.PHONY: all atomgen iridium
.SILENT: $(OBJS) $(OBJECTS)/iridium.o $(OBJECTS)/core.o atomgen

