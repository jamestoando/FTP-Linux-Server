 /* 
 * mftp.c
 *
 * Name: Toan Minh Do
 */

#include "mftp.h"

#define PORT_NUMBER 49999

struct in_addr **pptr;
struct sockaddr_in servAddr;
struct hostent* hostEntry;
int flag = -1;

// Check the connection and set up socket
int establishConnection(char* servID, int portNum) {
	int fd;
	fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd < 0) {
		fprintf(stderr, "[fd err code: %d] - %s \n", errno, strerror(errno));
		return -1;
	}
	else {
		fd = socket(AF_INET, SOCK_STREAM, 0);
	}

	memset(&servAddr, 0, sizeof(servAddr));
  	servAddr.sin_family = AF_INET;
  	servAddr.sin_port = htons(portNum);
  	hostEntry = gethostbyname(servID);

  	if(!hostEntry) {
  		fprintf(stderr, "No hostname specifiled! \n[hostEntry err code: %d] - %s \n", errno, strerror(errno));
  		return -1;
  	}

  	pptr = (struct in_addr **) hostEntry->h_addr_list;
  	memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));

  	if((connect(fd, (struct sockaddr*) &servAddr, sizeof(servAddr))) < 0) {
  		fprintf(stderr, "Connection failed! \n[fd err code: %d] - %s \n", errno, strerror(errno));
  		return -1;
  	}
  	return fd;
}

// Change dir
void cd(char* input) {
	int path;
	path = chdir(input);
	if(path < 0) {
		fprintf(stderr, "Unable to change directory! [err code: %d] - %s \n", errno, strerror(errno));
	}
	else {
		printf("\n");
	}
}

// Manipulates and returns the buffer
char* getMsg(int socket) {
	char* buf = malloc(450);
	register int n;
	char check;
	read(socket, &check, 1);
	for(n = 0; check != '\n'; n++) {
		buf[n] = check;
		read(socket, &check, 1);
	}
	buf[n] = '\0';
	return buf;
}

// Transfer file from server
int put(int datafd, int socketfd, char* word, char* path){
	// defining needed variables for open/write
	int fd;
	int openRead;
	char* buffer;
	char* newBuffer;

	if((fd = open(path, O_RDONLY, 0600)) < 0) {
		fprintf(stderr, "ERROR: Put error\n"); //, strerror(errno));
		return -1;
	}
	write(socketfd, word, strlen(word));
	buffer = getMsg(socketfd);
	if (buffer[0] == 'E'){
		newBuffer = strcat(buffer+1, "\0");
		printf("%s\n", newBuffer);
		return -1;
	} 
	else {
		while((openRead = read(fd, buffer, 450)) > 0) {
			write(datafd, buffer, openRead);
		}
	}
	close(fd);
	printf("\n");
	return 0;
}	

// Function for listing client's directory
void ls() {
	int parent = 1;
	if((parent = fork())) {
		wait(&parent);
		printf("\n");
	}
	else {
		int fd[2];
		pipe(fd);
		int child = 1;
		int reader = fd[0];
		int writer = fd[1];

		if((child = fork())) {
			close(writer);
			close(0);
			dup(reader);
			close(reader);
			wait(&child);
			execlp("more", "more", "-20", NULL);
		}
		else {
			close(reader);
			close(1);
			dup(writer);
			close(writer);
			execlp("ls", "ls", "-l", NULL);
		}
	}
}

// Remote ls
void rls(int fd) {
	int parent = 1;
	if((parent = fork())) {
		wait(&parent);
		printf("\n");
	}
	else {
		close(0);
		dup(fd);
		execlp("more", "more", "-20", NULL);
	}
}

// Gets file from server
void get(int fd, char* input) {
	int file;
	int reader;
	char buf[450];

	if((file = open(input, O_WRONLY | O_CREAT | O_EXCL, 0600)) < 0) {
		if(flag == -1) {
			fprintf(stderr, "File already exist! [err code: %d] - %s \n", errno, strerror(errno));
		}
		return;
	}

	flag = -1;
	while((reader = read(fd, buf, 450))) {
		write(file, buf, reader);
	}
	close(file);
	printf("\n");
	return;
}

int main(int argc, char **argv) {
	char* buf;
	char* clientCMD = NULL;
	char  serverCMD[450];
	char *path;
	char input[450];
	int fd = 0;

	if(argc < 2) {
		printf("Please enter a hostname!");
		exit(EXIT_FAILURE);
	}

	char* serverID = argv[1];
	int socketfd;
	socketfd = establishConnection(serverID, PORT_NUMBER);

	if(socketfd < 0) {
		return -1;
	}
	while(1) {
		printf("Enter a command (type 'help' to see commands): ");
		fgets(input, 450, stdin);
		printf("\n");

		if(input[0] == EOF) {
			break;
		}
		if(input[0] == '\n') {
			continue;
		}
		//printf("%s", input);

		// divides the string into tokens. i.e. starting 
		// from any one of the delimiter to next one would be your one token.
		clientCMD = strtok(input, " \n\t\v\f\r");
		// Receive 'exit' cmd
		if(!(strcmp(clientCMD, "exit"))) {
			write(socketfd, "Q\n", 2);
			buf = getMsg(socketfd);
			if(buf[0] == 'A') {
				printf("Disconnected from server.\n");
				return 0;
			}
		}

		// Start of if chain for receiving user's commands
		// Receive 'help' cmd
		if(strcmp(clientCMD, "help") == 0) {
			printf("*** Command List ***\n");
			printf("-> exit\n");
			printf("-> cd <pathname>\n");
			printf("-> rcd <pathname>\n");
			printf("-> ls\n");
			printf("-> rls\n");
			printf("-> get <pathname>\n");
			printf("-> show <pathname>\n");
			printf("-> put <pathname>\n");
		}
		// Receive 'cd' cmd
		else if(strcmp(clientCMD, "cd") == 0) { 
			path = strtok(NULL, " \n\t\v\f\r");
			cd(path);
			fflush(stdout);
		}
		// Receive 'ls' cmd
		else if(strcmp(clientCMD, "ls") == 0) {
			ls();
			fflush(stdout);
		}
		// Receive 'rcd' cmd
		else if(strcmp(clientCMD, "rcd") == 0) {
			fflush(stdout);
			path = strtok(NULL, " \n\t\v\f\r");
			if(!path) {
				printf("ERROR: path invalid!\n");
				continue;
			}
			strcpy(serverCMD, "C");
			strcat(serverCMD, path);
			strcat(serverCMD, "\n");
			//printf("%s", serverCMD);
			int len = strlen(serverCMD);
			write(socketfd, serverCMD, len);
			buf = getMsg(socketfd);
			//printf("%s", buf);
			fflush(stdout);
			if(buf[0] == 'E') {
				printf("%s\n", strcat(buf + 1, "\n"));
			}
		}
		// Receive 'cd' cmd
		else if(strcmp(clientCMD, "rls") == 0) {
			write(socketfd, "D\n", 2);
			buf = getMsg(socketfd);
			fflush(stdout);
			assert(buf);
			if(buf[0] == 'E') {
				printf("%s\n", strcat(buf+1, "\n"));
			}
			else {
				fd = establishConnection(serverID, atoi(buf+1));
				if(fd == -1) {
					return -1;
				}
				write(socketfd, "L\n", 2);
				buf = getMsg(socketfd);
				assert(buf);
				if(buf[0] == 'A') {
					assert(fd);
					rls(fd);
				}
				else {
					printf("ERROR: %s\n", strcat(buf+1, "\n"));
				}
			}
		}
		// Receive 'cd' cmd
		else if(strcmp(clientCMD, "show") == 0) {
			fflush(stdout);
			write(socketfd, "D\n", 2);
			buf = getMsg(socketfd);

			if(buf[0] == 'E') {
				printf("%s\n", strcat(buf + 1, "\0"));
			}
			else {
				fd = establishConnection(serverID, atoi(buf+1));
				path = strtok(NULL, " \n\t\v\f\r");
				if(!path) {
					printf("ERROR: 'show'\n");
					continue;
				}
				strcpy(serverCMD, "G");
				strcat(serverCMD, path);
				strcat(serverCMD, "\n");
				write(socketfd, serverCMD, strlen(serverCMD));
				buf = getMsg(socketfd);

				if(buf[0] == 'A') {
					flag = 1;
					get(fd, path);
					rls(fd);
				}
				else {
					printf("ERROR: %s\n", strtok(buf+1, "\n"));
				}
			}
		}
		// Receive 'get' cmd
		else if(strcmp(clientCMD, "get") == 0) {
			write(socketfd, "D\n", 2);
			buf = getMsg(socketfd);
			fflush(stdout);

			if(buf[0] == 'E') {
				printf("%s\n", strcat(buf + 1, "\n"));
			}
			else {
				fd = establishConnection(serverID, atoi(buf + 1));
				path = strtok(NULL, " \n\t\v\f\r");
				if(!path) {
					printf("ERROR: 'get' \n");
					continue;
				}
				strcpy(serverCMD, "G");
				strcat(serverCMD, path);
				strcat(serverCMD, "\n");
				write(socketfd, serverCMD, strlen(serverCMD));
				buf = getMsg(socketfd);
				if(buf[0] == 'A') {
					flag = -1;
					assert(fd);
					assert(path);
					get(fd, path);
					close(fd);
				}
				else {
					printf("ERROR: %s\n", strtok(buf + 1, "\n"));
				}
			}
		}
		// Receive 'put' cmd
		else if(strcmp(clientCMD, "put") == 0) {
			printf("%s", buf+1);
			fflush(stdout);
			write(socketfd, "D\n", 2);
			printf("%s", buf + 1);
			fflush(stdout);
			buf = getMsg(socketfd);
			if(buf[0] == 'E') {
				printf("%s\n", strcat(buf + 1, "\n"));
			}
			else {
				fd = establishConnection(serverID, atoi(buf+1));
				path = strtok(NULL, " \n\t\v\f\r");
				if(!path) {
					printf("ERROR: 'put'\n");
					continue;
				}
				strcpy(serverCMD, "P");
				strcat(serverCMD, path);
				strcat(serverCMD, "\n");
				put(fd, socketfd, serverCMD, path);
				close(fd);
			}
		}
		else {
			fprintf(stderr, "ERROR: Please enter a valid command.\n");
		}
	}
	free(buf);
	return 0;
}
