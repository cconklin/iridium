OBJS := object.o string.o dictionary.o file.o float.o iridium.o \
		lib/array.o lib/dict.o lib/linked_list.o lib/stack.o \
		lib/str_dict.o queue.o

iridium.a: $(OBJS) core.o
	ar rvs $@ $^

$(OBJS): %.o: %.c
	clang -o $@ -c $^

core.o: $(wildcard iridium/*.ir)
	ircc -o $@ -c iridium/core.ir --main=IR_CORE_INIT

clean:
	rm -f $(OBJS) core.o
	rm -f iridium.a

