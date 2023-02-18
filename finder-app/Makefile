CC := $(CROSS_COMPILE)gcc
all: writer.o

writer: writer.o
	$(CC) -o writer writer.o

writer.o: writer.c
	$(CC) -c writer.c

clean:
	rm -f *.o writer
