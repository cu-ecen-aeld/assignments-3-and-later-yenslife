CC := $(CROSS_COMPILE)gcc
all: writer

writer.o: writer.c
	$(CC) -c writer.c

writer: writer.o
	$(CC)  writer.o -o writer

clean:
	rm -f writer.o writer

