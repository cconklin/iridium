C_TESTS_SRC := $(shell find . -name "*.c" -type f)
IR_TESTS_SRC := $(shell find . -name "*.ir" -type f)

C_TESTS := $(C_TESTS_SRC:%.c=%)
IR_TESTS := $(IR_TESTS_SRC:%.ir=%)

ROOT := ..
IRIDIUM := $(ROOT)/iridium/objects/iridium.o
COMPILER_SRC := $(shell find $(ROOT)/ircc -name "*.rb" -type f)
COMPILE := $(ROOT)/scripts/compile_test

all: $(C_TESTS) $(IR_TESTS)

clean:
	rm -f $(C_TESTS)
	rm -f $(IR_TESTS)

$(C_TESTS): %: %.c $(IRIDIUM)
	$(COMPILE) $@.c

$(IR_TESTS): %: %.ir $(IRIDIUM) $(COMPILER_SRC)
	$(COMPILE) $@.ir

$(IRIDIUM):
	cd ../iridium && $(MAKE)

.SILENT:

