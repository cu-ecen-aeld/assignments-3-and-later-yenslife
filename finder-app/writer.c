// basic utility
#include<stdio.h>
#include<string.h>
//#include<errno.h>

// logging
#include<syslog.h>

// for open()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// for read() and write(2)
#include <unistd.h>

// for error handling
#include<errno.h>

void my_write(const char* writefile, char* writestr){
	// set file descriptor
	int fd = open( writefile, O_WRONLY | O_CREAT, 0777);
	if (fd < 0) {
		perror("open() error");
		syslog(LOG_ERR, "open() error: %s", strerror(errno));
		return;
	}
	write(fd, writestr, strlen(writestr));
	close(fd);
}


int main (int argc, char *argv[]) {
	
	openlog(NULL, 0, LOG_USER);
	if (argc != 3) {
		syslog(LOG_ERR, "Invalid number of arguments: %d, you should pass [file path] [message]", argc);
		return 1;
	} else {
		my_write(argv[1], argv[2]);
		syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
	}
	printf("test: end process successfully!\n");
	return 0;
}
