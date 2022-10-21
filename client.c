#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include  "request_response_data.h"

#define DEST_PORT            2000
#define MAX 80
#define SERVER_IP_ADDRESS   "127.0.0.1"

client_request_d client_data;
server_response_d server_result;

void tcp_communication(int s_key, int port, int time , int speed, char* ipaddr, char* key){

    /*Initialization*/   
    int sockfd = 0, 
        sent_recv_data= 0;

    int addr_len = 0;

    addr_len = sizeof(struct sockaddr);

    struct sockaddr_in dest;

    /* server information*/   
    dest.sin_family = AF_INET;

    /*Client wants  send data to server process */
 
    dest.sin_port = port;
    struct hostent *host = (struct hostent *)gethostbyname(ipaddr);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    /*Create a TCP socket*/  
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    /*recive data to be sent to server*/    


    

    for (int i = 0; i < speed*time; i++){
        if (i%100 == 0){
            printf("%d files encrypted\n", i);
        }
        client_data.buff[4] = (char)(s_key/1000)+48;
        client_data.buff[5] = (char)(s_key/100)-(s_key/1000)*10+48;
        client_data.buff[6] = (char)(s_key/10)-(s_key/100)*10+48;
        client_data.buff[7] = (char)(s_key) - (s_key/10)*10 +48;

        for (int i = 0; i < s_key*s_key; i++){
            client_data.buff[i+8] = key[i];
        }

        client_data.buff[0] ='0';
        client_data.buff[1] = (char) rand()%10+48;
        client_data.buff[2] = (char)  rand()%10+48;
        client_data.buff[3] = (char) rand()%10+48;


        //printf("Enter request file index, key : \n");  

        //scanf("%[^\n]%*c", client_data.buff);  
        //printf("data send : %s\n", client_data.buff);  
        
        /*
        printf("Enter index : ?\n");
        scanf("%u", &client_data.file_index);
        
        printf("Enter key : ?\n");
        scanf("%u", &client_data.key);
        printf("Data %s\n",server_result.reslt);
        
    */     

    

        /*send the data to server*/
        
        sent_recv_data = sendto(sockfd,&client_data,sizeof(client_request_d),0, (struct sockaddr *)&dest,sizeof(struct sockaddr));
            
    
        //printf("No of bytes sent = %d\n", sizeof(client_data));  
    
        
        /*recvfrom data from server*/
        sent_recv_data =  recvfrom(sockfd, (char *)&server_result, sizeof(server_response_d), 0,(struct sockaddr *)&dest, &addr_len);

        //printf("No of bytes recived = %d\n", sizeof(server_response_d));
        int x =strlen(server_result.reslt);
        //printf("Recived file size = %d\n",x);
        //printf("Recived encrypted file  = %s\n", server_result.reslt);

        /*If client to send data agin*/
        sleep(1/speed);
    }
   




}
    

int
main(int argc, char **argv){
    int time = 5;
    int speed = 100;
    int s_key = 2;
    int s = strlen(argv[argc-1]);
    char* str = malloc(s*sizeof(char));
    char* server = malloc((s-5)*sizeof(char));
    char* p = malloc(sizeof(char)*4);
    int port = 2000;
    strcpy(str, argv[argc-1]);
    
    strncpy(server, str, (s-5)*sizeof(char));

    int opt;
    while((opt = getopt(argc, argv, "k:r:t:?")) != -1){
        switch(opt){
            case 'k':
                s_key = atoi(optarg);
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
    char* key = malloc(s_key*s_key*sizeof(char));
    for(int i = 0;i < s_key*s_key; i++){
        key[i] = 48+rand()%10;
    }
    //printf("%s\n", key);
    tcp_communication(s_key, port, time, speed, server, key);
    printf("application quits\n");
    return 0;
}