 /* 
 * mftpserve.c
 *
 * Name: Toan Minh Do
 */
#include "mftp.h"

#define PORT_NUMBER 49999 

struct sockaddr_in servAddr;
struct sockaddr_in clientAddr;

char* portGrab;
char buffferChar;				
char found;						
char buffer[450];				
char reply[450];							
int listendatafd = 0;	
int datafd = 0;		

// Establish data connection 
int establishConnection(int fd) {
	int connectfd;
	struct sockaddr_in clientAddr;
	unsigned int length = sizeof(struct sockaddr_in);
	connectfd = accept(fd, (struct sockaddr *) &clientAddr, &length);
	return connectfd; 
}

// Reads and manipulates buffer, asserts that buffer is not null
char* bufferReader(char input, char* buffer, int connectfd) {
	register int n = 0;						   	
	while(input != '\n'){            		
		buffer[n] = input;					
		n++;								
		read(connectfd, &input, 1);			
	}
	buffer[n] = '\0';   
	assert(buffer);     
	return buffer;	    
}

// Function for setting up listen socket
int socketListener(int port) {
	int listenfd;
	int crashFixer;
	int crashFlag = 1;
	socklen_t optimum = sizeof(1);
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	crashFixer = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*) &crashFlag, optimum);
	return listenfd;
}

// Create server address
struct sockaddr_in makeServer(int port) {
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family =  AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	return servAddr;
}

// Fetch the port
char* getPort(int fd, struct sockaddr_in servAddr) {
	// Bidning operations
	if(bind(fd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		fprintf(stderr, "Binding operation failed! \n[bind err code: %d] - %s \n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	char* portNum;
	int getName;
	int port;
	portNum = malloc(16);		//length of port = 16
	struct sockaddr_in portSocket;
	unsigned int size = sizeof(portSocket);
	getName = getsockname(fd, (struct sockaddr *) &portSocket, &size);
	assert(getName != -1);
	port = ntohs(portSocket.sin_port);
	sprintf(portNum, "%d", port);
	return portNum;
}

// Change server's directory
int rcd(char* input) {
	// Check for new line in input
	strtok(input, "\n");
	// Path checker
	if(chdir(input) == -1) {
		printf("ERROR: The path is invalid!\n");
		return -1;
	}
	printf("%s\n", input);
	printf("The current directory has been changed!\n");
	return 0;
}

// List server's current directory
void rls(int fd) {
	int child;
	if(!(child = fork())) {
		close(1);
		dup(fd);
		if(execlp("ls", "ls", "-l", NULL)) {
			fprintf(stderr, "ERROR: Unable to list directory [err code: %d] - %s \n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	printf("The current directory has been read by server's terminal\n");
	wait(&child);
	return;
}

// Function for getting a file from the server and save it in client's directory
void get(int fd, int connectfd, char* input) {
	int file;
	int bytes;
	char buf[450];

	if((file = open(input, O_RDONLY, 0600)) < 0) {
		write(connectfd, "ERROR: 'get' failed!\n", 20);
		return;
	}
	write(connectfd, "A\n", 2);

	while((bytes = read(file, buf, 450)) > 0) {
		write(fd, buf, bytes);
	}
	close(file);
	return;
}

// Function for putting a file from the client to the server
int put(int fd, int connectfd, char* input) {
	int file;
	int bytes;
	char buf[450];

	if((file = open(input, O_WRONLY | O_CREAT | O_EXCL, 0600)) < 0) {
		fprintf(stderr, "ERROR: 'put' failed! [err code: %d] - %s \n", errno, strerror(errno));
		write(connectfd, "ERROR: 'put' failed!\n", 20);
		return -1;
	}
	write(connectfd, "A\n", 2);
	while((bytes = read(fd, buf, 450)) > 0) {
		write(file, buf, bytes);
	}
	close(file);
	return 0;
}


int main(int argc, char** argv) {
	int port = 0;	
	printf("Server is now online! Running...\n");
	if(argv[1]) {
		port = atoi(argv[1]);
	}
	if(argv[1] == NULL) {
		port = PORT_NUMBER;
	}

	int listenfd = socketListener(port);
	struct sockaddr_in servAddr = makeServer(port);
	struct sockaddr_in dataAddr;

	if(bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {  // check for bind error
		fprintf(stderr, "Binding operation failed! \n[bind err code: %d] - %s \n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	listen(listenfd, 4);
	socklen_t length = sizeof(struct sockaddr_in);
	struct sockaddr_in clientAddr;
	struct hostent* hostEntry;
	char* hostName;

	while(1) {
		int child;
		int connectfd;
		
		connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
		if(connectfd < 0) {
			fprintf(stderr, "ERROR: Connection failed! [err code: %d] - %s \n", errno, strerror(errno));
			return -1;
		}

		if(!(child = fork())) {
			hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET);
			hostName = hostEntry->h_name;
			printf("Child: %d\n", getpid());
			printf("Connection was established from %s\n", hostName);
			while(1) {
				if(!read(connectfd, &buffferChar, 1)) {
					printf("The connection is interrupted!\n");
					return 0;
				}
				bufferReader(buffferChar, buffer, connectfd);

				// Q = quit
				if(buffer[0] == 'Q') {
					printf("Client %s has disconnected.\n", hostName);
					write(connectfd, "A\n", 2);
					exit(EXIT_FAILURE);
				}

				// G = get a file
				if(buffer[0] == 'G') {
					if(listendatafd == -1) {
						printf("ERROR: Data connection failed!\n");
					}
					else {
						get(datafd, connectfd, buffer+1);
						close(datafd);
						close(listendatafd);
						// Could improve
						// datafd = 0;
						// listendatafd = datafd;
					}
				}

				// establish connection
				if(buffer[0] == 'D') {
					listendatafd = socketListener(port);
					dataAddr = makeServer(0);
					portGrab = getPort(listendatafd, dataAddr);
					strcpy(reply, "A");
					strcat(reply, portGrab);
					strcat(reply, "\n");
					write(connectfd, reply, (int) strlen(reply));
					listen(listendatafd, 1);
					datafd = establishConnection(listendatafd);
				}

				// change dir
				if(buffer[0] == 'C') {
					printf("%s", buffer);
					if((rcd(buffer + 1) != -1)) {
						write(connectfd, "A\n", 2);
					}
					else {
						write(connectfd, "ERROR: 'rcd' failed!\n", 13);
					}
				}

				// rls
				if(buffer[0] == 'L') {
					if(!listendatafd) {
						printf("ERROR: Data connection failed!\n");
					}
					else {
						rls(datafd);
						write(connectfd, "A\n", 2);
						close(datafd);
						close(listendatafd);
						// Could improve this
						// datafd = 0;
						// listendatafd = datafd;	
					}
				}

				// put
				if(buffer[0] == 'P') {
					if(!listendatafd) {
						printf("ERROR: Data connection failed\n");
					}
					else {
						put(datafd, connectfd, buffer+1);
						close(datafd);
						close(listendatafd);
						//
						// datafd = 0;
						// listendatafd = datafd;
					}
				}
			}
		}
		close(connectfd);
	}
	free(portGrab);
	return 0;

}

