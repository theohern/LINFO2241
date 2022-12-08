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
#include <sys/time.h>

#define ARRAY_TYPE float
int npages = 1000;

struct timeval begin, finish;

typedef struct arg
{
    ARRAY_TYPE* key;
    int i;
    int keysz;
    int* receive_times;
    int port;
}arg_d;

void * rcv(void* r){
    arg_d* argument = (arg_d*) r;
    ARRAY_TYPE* key = argument->key;
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


    gettimeofday(&begin, NULL);


    int a = rand()%npages;
    //unsigned fileindex = htonl(a);

    ret = send(sockfd, &a, 4, 0);

    //int revkey = htonl(keysz);
    ret = send(sockfd, &keysz, 4, 0);

    ret = send(sockfd, key, sizeof(ARRAY_TYPE) * keysz * keysz, 0);

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
    gettimeofday(&finish, NULL);

    long seconds = (finish.tv_sec - begin.tv_sec);
    long micros = ((seconds * 1000000) + finish.tv_usec) - (begin.tv_usec);
    argument->receive_times[argument->i] = (int) micros;
    close(sockfd);
}
    
int mean (int* tab, int size){
    int sum = 0;
    for (int i = 0; i < size; i++){
        sum+=tab[i];
    }
    return sum/size;
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

    
    ARRAY_TYPE* key = malloc(sizeof(ARRAY_TYPE) * keysz * keysz);
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




    pthread_t thread_pool[rate*times];

    for (int i = 0; i < rate*times; i++){
        pthread_create(&thread_pool[i], NULL, rcv, argument);
    }

    int start = time(NULL);
    int temp = time(NULL);
    i = 0;
    double diffrate = 1/rate;
    int ok = 1;

    while ((time(NULL) - start) < times)
    {
        if (ok){
            for(int message = 0; message < rate; message++){
                argument->i = i;
                pthread_create( &thread_pool[i], NULL, rcv, (void*) argument);
                i++;
            }
            ok = 0;
        } else if ((time(NULL) - temp) != 0){
            ok = 1;
            temp = time(NULL);
        }
    } 

    //printf("%d threads launched\n", i);     

    for (int i = 0; i < rate*times; i++){
        pthread_join(thread_pool[i], NULL);
    }  

    int end = time(NULL);
   
    
    // printf("number of request : %d\n", i);
    // printf("application quits\n");
    printf("%d\n", mean(argument->receive_times, rate*times));

    free(key);
    free(argument->receive_times);
    free(argument);
    return 0;

}
