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
#include <signal.h>
#include <errno.h>



#define PORT "9000"
#define BACKLOG 10
#define FILEPATH "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 2000000
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

    //printf("read file buffer: %s\n", read_buffer);

    return read_buffer;

}

bool connecting;
int socket_fd;
int new_socket_fd;


// signal handle
void signal_handler(int sig_num)
{
    if(sig_num == SIGINT || sig_num == SIGTERM)
    {
        syslog(LOG_USER, "Caught signal, exiting");
	if (!remove(FILEPATH)) {
            printf("delete FILEPATH ok\n");
	}
	close(socket_fd);
	close(new_socket_fd);
        shutdown(socket_fd, SHUT_RDWR);
	shutdown(new_socket_fd, SHUT_RDWR);
        connecting = false;
	exit(sig_num);
    }
}

int main (int argc, char *argv[]) {

    // signal setup
    struct sigaction new_action;
    memset(&new_action,0,sizeof(struct sigaction));

    new_action.sa_handler=signal_handler;
    if( sigaction(SIGTERM, &new_action, NULL) != 0 ) {
        printf("Error %d (%s) registering for SIGTERM",errno,strerror(errno));
    }
    if( sigaction(SIGINT, &new_action, NULL) ) {
        printf("Error %d (%s) registering for SIGINT",errno,strerror(errno));
    }
    
    // daemon check
    bool daemon = false;
    if (argc >= 2 && !(strcmp(argv[1], "-d"))) {
        daemon = true; // rememver to fork after bind
    }
    
    
    /* define socketfd and addrinfo result and hints  */ 
    
    socket_fd = socket(PF_INET , SOCK_STREAM, 0);
    
    struct addrinfo *result, hints;

    int addrinfo_status;

    /* see `man getaddrinfo` example  */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* STREAM socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    
    addrinfo_status = getaddrinfo(NULL, PORT, &hints, &result); 
    if (addrinfo_status != 0) // note that result is pointer to pointer
    {
        fprintf(stderr, "getaddrinfo error: %d\n", addrinfo_status);
	perror("error when getaddrinfo");
	exit(1);
    }

    

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


    freeaddrinfo(result);

    if (daemon) {
	    printf("daemon mode\n");
	    pid_t pid;

	    /* Fork off the parent process */
	    pid = fork();

	    /* An error occurred */
	    if (pid < 0) {
	        close(socket_fd);
		perror("fork error");
		exit(EXIT_FAILURE);
	    }

	    /* Success: Let the parent terminate */
	    if (pid > 0)
		exit(EXIT_SUCCESS);

	    /* On success: The child process becomes session leader */
	    if (setsid() < 0) {
	        close(socket_fd);
		exit(EXIT_FAILURE);
	    }


	    /* Set new file permissions */
	    //umask(0);

	    /* Change the working directory to the root directory */
	    /* or another appropriated directory */
	    int chdir_status = chdir("/");

	    if (chdir_status == -1) {
	        close(socket_fd);
		perror("chdir error");
		exit(EXIT_FAILURE);
	    }
	    /* Close all open file descriptors */
	    /*int x;
	    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
	    {
		close (x);
	    }*/
	    /*open("/dev/null", O_RDWR);
	    dup(0);
	    dup(0);*/

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

    connecting = true;
    char *buffer;// = malloc(BUFFER_SIZE * sizeof(char));
    while (connecting) {
    // accept 
        new_socket_fd = accept(socket_fd, result->ai_addr, &(result->ai_addrlen));
	if (new_socket_fd != -1) {
	    syslog(LOG_USER, "Accepted connection from %s", inet_ntop(AF_INET, &result, client_ip, INET_ADDRSTRLEN));
	    printf("Accept connection from:  %s\n",  inet_ntop(AF_INET, &result, client_ip, INET_ADDRSTRLEN));
	    
	}
	if (new_socket_fd == -1) {
            continue;
	}

	// read value

	ssize_t read_val;
	
	buffer = malloc(BUFFER_SIZE * sizeof(char));

	memset(buffer, 0, BUFFER_SIZE);


	while((read_val = recv(new_socket_fd, buffer, BUFFER_SIZE, 0)) > 0){
		//int read_val = recv(new_socket_fd, buffer, BUFFER_SIZE, 0);
		//read_val = read(new_socket_fd, buffer, BUFFER_SIZE)

	//	char *data = malloc(strlen(buffer) + 2);

		if (read_val == -1) {
			printf("read error\n");
			perror("read error");
			exit(1);
		}

		write_file(FILEPATH, buffer, read_val);



		/*for (int i = 0; i < strlen(buffer); i++) {
			data[i] = buffer[i];

			if (buffer[i] == '\0') {
				exit(0);
			}

			if(buffer[i] == '\n') {
				printf("find new line\n");
				data[i] = '\n';
				//data[i + 1] = '\0';
				break;
			}
		}*/
		printf("read value: %s", buffer); // test


		// read file before send
		char *data_send;
		data_send = read_file(FILEPATH, read_val);
		int send_status = send(new_socket_fd, data_send, strlen(data_send), 0);
		//free(data);
		free(data_send);

		if (send_status == -1) {
			perror("send_status error: ");
			exit(1);
		}


	}
	free(buffer);
	syslog(LOG_USER, "Closed connection from %s", inet_ntop(AF_INET, &result, client_ip, INET_ADDRSTRLEN));
        printf("Closed connectino from %s\n\n\nwaiting...\n", inet_ntop(AF_INET, &result, client_ip, INET_ADDRSTRLEN));
	close(new_socket_fd);
	shutdown(new_socket_fd, SHUT_RDWR);
	//freeaddrinfo(result);

    }
    //freeaddrinfo(result);
    remove(FILEPATH);
    close(socket_fd);
    shutdown(socket_fd, SHUT_RDWR);


       


    return 0;
}
