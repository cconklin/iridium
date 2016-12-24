OBJS := object.o string.o dictionary.o file.o float.o iridium.o \
		lib/array.o lib/dict.o lib/linked_list.o lib/stack.o \
		lib/str_dict.o

iridium.a: $(OBJS)
	ar rvs $@ $^

%.o: %.c
	clang -o $@ -c $^

clean:
	rm -f $(OBJS)
	rm -f iridium.a

