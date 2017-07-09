SRCS 	:= $(shell find src -name "*.c")
OBJS 	:= $(SRCS:src/%.c=objects/%.o)
IR   	:= $(wildcard iridium/*.ir)
CC   	:= clang
CFLAGS 	:= $(shell pkg-config --cflags bdw-gc) -I./include/
LDFLAGS := $(shell pkg-config --libs bdw-gc)
OBJECTS := objects
# Can add -g flag to .c and .ir rules to get debug symbols

all: $(OBJECTS) $(OBJECTS)/ir.o

$(OBJECTS)/ir.o: $(OBJS) $(OBJECTS)/core.o
	@echo LD $@
	ld -r -o $@ $(OBJS) $(OBJECTS)/core.o

$(OBJS): $(OBJECTS)/%.o: src/%.c
	@echo CC $@
	$(CC) $(CFLAGS) -o $@ -c $< -Wall

$(OBJECTS):
	mkdir $(OBJECTS)
	mkdir $(OBJECTS)/shared

$(OBJECTS)/core.o: $(IR)
	@echo IR $@
	ircc -o $@ -c iridium/core.ir --main=IR_CORE_INIT

clean:
	rm -rf $(OBJECTS)

.SILENT: $(OBJS) $(OBJECTS)/ir.o $(OBJECTS)/core.o

