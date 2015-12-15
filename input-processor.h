#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>

void CreateInputProcess(pthread_t* p, int socketID);

void *Input(void *command);

void GetCmd(char* command, int length, int soc);

void Kill(int soc);

int SocketIsOpen(int soc);