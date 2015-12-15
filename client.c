#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define PORT 8765

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int soc, n;
    int portno = atoi(argv[2]);
    struct sockaddr_in serverAddress;
    struct hostent *server;

    soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serverAddress.sin_addr.s_addr,
         server->h_length);
    serverAddress.sin_port = htons(portno);

    if (connect(soc,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
        error("ERROR connecting");

    // int i = 0;
    // for (i = 0; i < 2; ++i) {
    //     char* socketCommand = "ADD hello.txt 3\nABC";
    //     send(soc, socketCommand, strlen(socketCommand), 0);
    // }

    while( 1 ) {         
        printf("Please enter the message: ");
        char buffer[256];
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        n = send(soc,buffer,strlen(buffer), 0);
        if (n < 0) 
             error("ERROR writing to socket");
        // bzero(buffer,256);
        // n = read(sock,buffer,255);
        // if (n < 0) 
        //      error("ERROR reading from socket");
        // printf("%s\n",buffer);
    }

    close(soc);
    return 0;
}

//     char buffer[256];
//     if (argc < 3) {
//        fprintf(stderr,"usage %s hostname port\n", argv[0]);
//        exit(0);
//     }
//     portno = atoi(argv[2]);
    
//     server = gethostbyname(argv[1]);
//     if (server == NULL) {
//         fprintf(stderr,"ERROR, no such host\n");
//         exit(0);
//     }
//     bzero((char *) &serv_addr, sizeof(serv_addr));
    
    
//     while( 1 ) {
//         printf("Please enter the message: ");
//         bzero(buffer,256);
//         fgets(buffer,255,stdin);
//         n = write(sockfd,buffer,strlen(buffer));
//         if (n < 0) 
//              error("ERROR writing to socket");
//         bzero(buffer,256);
//         n = read(sockfd,buffer,255);
//         if (n < 0) 
//              error("ERROR reading from socket");
//         printf("%s\n",buffer);
//     }
//     return 0;
// }