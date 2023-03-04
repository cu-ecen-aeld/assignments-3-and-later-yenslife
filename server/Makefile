# CC = aarch64-none-linux-gnu-gcc

CC := $(CROSS_COMPILE)gcc

CFLAGS = -Wall -Werror

# CFLAGS = -Wall

all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.c

clean:
	rm -f aesdsocket /var/tmp/aesdsocketdata

