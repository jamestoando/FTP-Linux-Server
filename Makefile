#Makefile
#FTP-Linux-Server

#make: compiles the files using gcc with standard warnings.
#make clean: cleans up the .o files. 
#make tar: creates tar ball of all the files.

CC=gcc
CFLAGS=-g -std=c99 -pedantic -Wall -Werror

all: mftp mftpserve

mftp: mftp.c mftp.h
	$(CC) $(CFLAGS) -o mftp mftp.c -lm
mftpserve: mftpserve.c mftp.h
	$(CC) $(CFLAGS) -o mftpserve mftpserve.c -lm
clean:
	rm mftpserve mftp

.PHONY: all run

tar:
		tar -cvf ${PROJECT_NAME}.tgz ${FILES}
		clear
		@echo "Tar Ball ${PROJECT_NAME} created with the following files:\n${FILES}" 
