/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
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

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{

     int sockfd, newsockfd, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
    
 	 printf("Listening on port %d\n", portno);
     listen(sockfd,5);
     //clilen = sizeof(cli_addr);
     while(1) {
          newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
          
          if (newsockfd < 0) 
               error("ERROR on accept");
          printf("Received incoming connection from <%s>\n",
               inet_ntoa(cli_addr.sin_addr));
          // bzero(buffer,256);
          // n = read(newsockfd,buffer,255);
          // if (n < 0) error("ERROR reading from socket");
          // printf("Here is the message: %s\n",buffer);
          // n = write(newsockfd,"I got your message",18);
          // if (n < 0) error("ERROR writing to socket");
          
          static pthread_t thread;
          CreateInputProcess(&thread, newsockfd);
     }
     return 0; 
}