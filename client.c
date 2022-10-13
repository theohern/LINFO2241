#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}
   
int main(int argc, char *argv[])
{
    int time;
    int speed;
    int key;
    int s = strlen(argv[argc-1]);
    char* str = malloc(s*sizeof(char));
    char* server = malloc((s-5)*sizeof(char));
    char* p = malloc(sizeof(char)*4);
    int port = 0;
    strcpy(str, argv[argc-1]);
    
    strncpy(server, str, (s-5)*sizeof(char));

    int opt;
    while((opt = getopt(argc, argv, "k:r:t:?")) != -1){
        switch(opt){
            case 'k':
                key = atoi(optarg);
                break;
            case 'r':
                speed = atoi(optarg);
                break;
            case 't':
                time = atoi(optarg);
                break;
            default :
                printf("something goes wrong !");
        }
        
    }
    p = &str[strlen(str)-4];
    port = atoi(p);
    printf("time : %d\nspeed : %d\nkey : %d\nport : %d\ntcp : %s\n", time, speed, key, port, server);


    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
   
    // function for chat
    func(sockfd);
   
    // close the socket
    close(sockfd);
}