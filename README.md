# FTP-Linux-Server
## Created by: Toan (James) Minh Do

Allows two systems on the same network to view, modify and share files.

## Instructions

To compile the program, make sure the Makefile and .c files are in the same path. Commands for using make: 
1. make- Compiles the program 
2. make clean- removes the object files created during compiling
3. make tar- creates a tar ball of the files

Open Terminal on the client system and executes: $ ./mftp
Open Terminal on the server system and executes: $ ./mftpserve

The following commands are supported (this list is also available in the program by executing 'help'):
1. cd <pathname>: change the directory of the client system
2. rcd <pathname>: change the directory of the server system
3. ls: list all the files within the client's current directory
4. rls: list all the files within the server's current directory
5. get <pathname>: download a specified file from the server system to the client system
6. put <pathname>: upload a specified file from the client to the server
7. show <pathname>: view the content of a text file on the server

### Requirements
1. Linux/Unix operating system
2. GCC compiler
3. LAN connection between the server and the client systems

#### Package Contents
1. mftp.c
2. mftp.h
3. mftpserve.c
4. Makefile
5. README.md
