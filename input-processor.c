#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>

#include "input-processor.h"

void CreateInputProcess(pthread_t* p, int sID) {
	
	pthread_create(p, NULL, &input, &sID);
}

void *input(void *command) {
	
	int socket = *(int*) command;
	
	while (1) {
		char header[128];
		char fileName[128];
		
		GetCmd(header, 128, socket);
		
		char* temp = NULL;
		char* instructions = strtok_r(header, " \r\n", &temp);
		
		if (command == NULL) {
			fprintf(stderr, "We done fucked up, command is invalid.");
		}
		else {
			printf("Command inputted: %s", instructions);
		}
	}
}

void GetCmd(char* command, int length, int socket) {
	char bufChar = '\0';
	int i = 0;
	for (i = 0; i < length; ++i) {
		int j = read(socket, &buffer, 1);
		if (j <= 0) {
			Kill(socket);
			return;
		}
		if (buffer == '\n' || buffer == '\r') {
			break;
		}
		header[i] = buffer;
	}
	header[i] = '\0';
}

void Kill(int socket){
	if (SocketIsOpen(socket) == 1) {
		close(socket);
		pthread_exit(NULL);
	}
}

int SocketIsOpen(int socket) {
	int i = send(socket, '\0', 1, MSG_NOSIGNAL);
	if (i < 0) {
		return 0;
	}
	else {
		return 1;
	}
}