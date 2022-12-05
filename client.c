#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define DEST_PORT  8080
int npages = 1000;
typedef struct arg
{
    int32_t* key;
    int i;
    int keysz;
    int* receive_times;
    int port;
}arg_d;

void * rcv(void* r){
    arg_d* argument = (arg_d*) r;
    int32_t* key = argument->key;
    int keysz = argument->keysz;
    int port = argument->port;

    /*Initialization*/    
    int sockfd, ret;
    struct sockaddr_in servaddr;

    /*Create a TCP socket*/  
    //sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

     /* server information*/   
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(struct sockaddr))<0){
        printf("error to connect.. \n");
    }

    int a = rand()%npages;
    //unsigned fileindex = htonl(a);
    ret = send(sockfd, &a, 4, 0);

    //int revkey = htonl(keysz);
    ret = send(sockfd, &keysz, 4, 0);

    ret = send(sockfd, key, sizeof(int32_t) * keysz * keysz, 0);

    unsigned char error;
    recv(sockfd, &error, 1, 0);

    unsigned int filesz;
    recv(sockfd, &filesz, 4, 0); 

    int sz = 1024;
    char buffer[65536];

    if (filesz > 0) {
        
        long int left = sz;  
        while (left > 0) {
            unsigned b = left;
            if (b > 65536)
                b = 65536;
            left -=  recv(sockfd, &buffer, b, 0);
        }
    }


    unsigned t = argument->i;
    close(sockfd);
}
    


int
main(int argc, char **argv){
    int i = 0;
    int next = 0;
    int times = 5;
    int rate = 100;
    int keysz = 2;
    int s = strlen(argv[argc-1]);
    char* str = malloc(s*sizeof(char));
    char* server = malloc((s-5)*sizeof(char));
    char* p = malloc(sizeof(char)*4);
    int port = 8080;
    strcpy(str, argv[argc-1]);
    
    strncpy(server, str, (s-5)*sizeof(char));

    int opt;
    while((opt = getopt(argc, argv, "k:r:t:?")) != -1){
        switch(opt){
            case 'k':
                keysz = atoi(optarg);
                break;
            case 'r':
                rate = atoi(optarg);
                break;
            case 't':
                times = atoi(optarg);
                break;
            default :
                printf("something goes wrong !");
        }
        
    }
    p = &str[strlen(str)-4];
    port = atoi(p); 
    int sent_time[rate*times];

    
    int32_t* key = malloc(sizeof(int32_t) * keysz * keysz);
    for(i = 0; i < keysz*keysz; i++){
        key[i] = rand()%10;
    }
    // printf("keysz : %d\nrate : %d\ntime : %d\nport : %d\nip : %s\n", keysz, rate, times, port, server);
    // printf("key : ");
    // for (int j = 0; j < keysz*keysz; j++){
    //     printf("%d", key[j]);
    // }
    // printf("\n");

    arg_d* argument = malloc(sizeof(arg_d));
    argument->key = key;
    argument->keysz = keysz;
    argument->receive_times = malloc(sizeof(int) * rate * times);
    argument->port = port;


    int start = time(NULL);
    int temp = time(NULL);

    i = 0;
    double diffrate = 1/rate;
    int count = 0;
    int ok = 1;

    while (time(NULL) - start < times)
    {
        if (ok){
            for(int message = 0; message < rate; message++){
                argument->i = i;
                pthread_t thread; 
                pthread_create( &thread, NULL, rcv, (void*) argument);
                i++;
                count ++;
            }
        }
        ok = 0;
        if (time(NULL) - temp != 0){
            ok = 1;
            count = 0;
            temp = time(NULL);
            
        }
    }        

   
    
    printf("number of request : %d\n", i);
    printf("application quits\n");
    free(key);
    free(argument->receive_times);
    free(argument);
    return 0;

}
