CC := $(CROSS_COMPILE)gcc

writer: writer.o
	$(CC) -o writer writer.o

writer.o: writer.c
	$(CC) -c writer.c
#writer: writer.o
#	$(CC) -o writer writer.o
clean:
	rm -f *.o writer
