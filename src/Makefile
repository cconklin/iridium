SRCS 	:= $(shell find . -name "*.c")
OBJS 	:= $(SRCS:%.c=%.o)
IR   	:= $(wildcard iridium/*.ir)
CC   	:= clang
CFLAGS 	:= $(shell pkg-config --cflags bdw-gc)
LDFLAGS := $(shell pkg-config --libs bdw-gc)

# Can add -g flag to .c and .ir rules to get debug symbols

ir.o: $(OBJS) core.o
	@echo LD $@
	ld -r -o $@ $^

$(OBJS): %.o: %.c
	@echo CC $@
	$(CC) $(CFLAGS) -o $@ -c $^ -Wall

core.o: $(IR)
	@echo IR $@
	ircc -o $@ -c iridium/core.ir --main=IR_CORE_INIT

clean:
	rm -f $(OBJS) core.o ir.o

.SILENT: $(OBJS) ir.o core.o

