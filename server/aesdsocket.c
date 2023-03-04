#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> // for inet_ntop to return client ip
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>


#define PORT "9000"
#define BACKLOG 10
#define FILEPATH "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 200000
//int socket(int domain, int type, int protocol);



// open FILEPATH file
// int open(const char *pathname, int flags);
void write_file(char* filename, char* buffer, ssize_t count)
{
    
    int fd = open(filename, (O_RDWR | O_CREAT | O_APPEND | O_DSYNC), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));

    if (fd == -1) {
        perror("open file error");
	exit(1);
    }
    int write_status = write(fd, buffer, count);

    if (write_status == -1) {
        perror("write error");
	exit(1);
    }


    close(fd);

}

// read file data in FILEPATH
char *read_buffer;
char *read_file(char *filename, ssize_t count)
{
    FILE *fp = fopen(FILEPATH, "r");

    if (fp == NULL) {
        perror("fopen error");
	exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET); 
    read_buffer = malloc(fsize + 1);
    fread(read_buffer, fsize, 1, fp);

    read_buffer[fsize] = 0;    

    fclose(fp);

    printf("read file buffer: %s\n", read_buffer);

    return read_buffer;

}

int main (int argc, char *argv[]) {
    
    /* define socketfd and addrinfo result and hints  */ 
    
    int socket_fd = socket(PF_INET , SOCK_STREAM, 0);
    
    struct addrinfo *result, hints;

    int addrinfo_status;

    /* see `man getaddrinfo` example  */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* STREAM socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    // other set up is 0 for default beacuase of we use memset to 0
    
    addrinfo_status = getaddrinfo(NULL, PORT, &hints, &result); 
    if (addrinfo_status != 0) // note that result is pointer to pointer
    {
        fprintf(stderr, "getaddrinfo error: %d\n", addrinfo_status);
	perror("error when getaddrinfo");
	exit(1);
    }
    // don't forget to use freeaddrinfo

    

    if (socket_fd == -1) {
        // error when open socket
	perror("error when open socket");
	exit(1);
    }
    
    // avoid Address already in use
    int optname = 1;
    if (setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&optname,sizeof optname) == -1) {
        perror("setsockopt");
        exit(1);
    } 	
    int bind_status = bind(socket_fd, result->ai_addr, result->ai_addrlen);
    if (bind_status == -1) {
        perror("Bind Error");
	exit(1);
    }
    
    int listen_status = listen(socket_fd, BACKLOG);
    if (listen_status == -1){
        perror("listen error");
	exit(1);
    }
    printf("test message before accept\n");
   
    char client_ip[INET_ADDRSTRLEN];
    // test
    //printf("test: %s",  inet_ntop(AF_INET, &result, client_ip, INET_ADDRSTRLEN));

    bool connecting = true;
    char *buffer = malloc(BUFFER_SIZE * sizeof(char));
    while (connecting) {
    // accept 
        int new_socket_fd = accept(socket_fd, result->ai_addr, &(result->ai_addrlen));
	if (new_socket_fd != -1) {
	    syslog(LOG_USER, "Accepted connection from %s", inet_ntop(AF_INET, &result, client_ip, INET_ADDRSTRLEN));
	    printf("test: %s\n",  inet_ntop(AF_INET, &result, client_ip, INET_ADDRSTRLEN));
	}

	// read value
	
	int read_val = read(new_socket_fd, buffer, BUFFER_SIZE);
        char *data = malloc(strlen(buffer) + 2);

	if (read_val == -1) {
	    perror("read error");
	    exit(1);
	}

	write_file(FILEPATH, buffer, read_val);



	for (int i = 0; i < strlen(buffer); i++) {
	    data[i] = buffer[i];
	    if(buffer[i] == '\n') {
	        printf("find new line\n");
		//connecting = false;
		data[i] = '\n';
		data[i + 1] = '\0';
		break;
	    }
	}
	printf("read value: %s", data); // test


	// read file before send
	char *data_send;
	data_send = read_file(FILEPATH, read_val);
        send(new_socket_fd, data_send, strlen(data_send), 0);
	//free(data);
	//free(data_send);


    }
    
    // send back to client
   


    freeaddrinfo(result);
    


    return 0;
}
