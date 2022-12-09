#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>

#define ARRAY_TYPE float
int npages = 1000;

// Function designed for chat between client and server.
void connection_handler(void *socket_desc,int nbytes, ARRAY_TYPE **pages)
{
    //printf("connction_handler begin\n");
    int fileid;
    int keysz;
    int sockfd = (int)(intptr_t)socket_desc;

    int tread = recv(sockfd, &fileid, 4, 0);
    fileid = ntohl(fileid);
    if (fileid >= 1000){
        printf("file id %d is more than 1000 !\n", fileid);
        exit(1);
    }
    //fileid = 5;
    tread = recv(sockfd, &keysz, 4, 0);
    //Network byte order
    keysz = ntohl(keysz);
    if (keysz != 8 && keysz != 128 && keysz !=1){
        printf("key size %d is different than 8 or 128 !", keysz);
        exit(1);
    }

    //printf("keysz=%d, fileid=%d\n", keysz, fileid);
    ARRAY_TYPE* key;
    key = malloc(keysz*keysz*sizeof(ARRAY_TYPE));
    unsigned tot = keysz*keysz * sizeof(ARRAY_TYPE);

    unsigned done = 0;
    while (done < tot) {
        tread = recv(sockfd, key, tot- done, 0);
        done += tread;
    }

    int nr = nbytes / keysz;
    ARRAY_TYPE* file = pages[fileid % npages];
    ARRAY_TYPE* crypted = malloc(nbytes*nbytes * sizeof(ARRAY_TYPE));
    //Compute sub-matrices

    for (int i = 0; i < nr ; i ++) {
        int vstart = i * keysz;
        for (int j = 0; j < nr; j++) {
            int hstart = j * keysz;

            //Do the sub-matrix multiplication
            for (int ln = 0; ln < keysz; ln++) {

                int aline = (vstart + ln) * nbytes + hstart;
                for (int k = 0; k < keysz; k++) {

                    ARRAY_TYPE a = key[ln * keysz + k];
                    int vline = (vstart + k) * nbytes + hstart;
                    for (int col = 0; col < keysz; col+=8) {
                        crypted[aline + col] +=  a * file[vline + col];
                        crypted[aline + col+1] +=  a * file[vline + col+1];
                        crypted[aline + col+2] +=  a * file[vline + col+2];
                        crypted[aline + col+3] +=  a * file[vline + col+3];
                        crypted[aline + col+4] +=  a * file[vline + col+4];
                        crypted[aline + col+5] +=  a * file[vline + col+5];
                        crypted[aline + col+6] +=  a * file[vline + col+6];
                        crypted[aline + col+7] +=  a * file[vline + col+7];
                    }

                }
            }
        }
    }





    int err = 0;
    send(sockfd, &err, 1,MSG_NOSIGNAL );
    unsigned sz = htonl(nbytes*nbytes * sizeof(ARRAY_TYPE));
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    send(sockfd, crypted, nbytes*nbytes * sizeof(ARRAY_TYPE),MSG_NOSIGNAL );
     printf("before free\n");

    //free(crypted);
    // free(key);
     printf("after free\n");


}

// Driver function
int main(int argc, char **argv)
{
    int opt;
    int port = 8080;
    int nthread = 4;
    int nbytes = 4;


    while((opt = getopt(argc, argv, "j:s:p:?")) != -1){
        switch(opt){
            case 'j':
                nthread = atoi(optarg);
                break;
            case 's':
                nbytes = atoi(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default :
                printf("something goes wrong !");
        }
    }
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
	memset(&servaddr,0, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

    ARRAY_TYPE **pages = malloc(sizeof(ARRAY_TYPE*) * npages);
    for (int i = 0 ; i < npages; i++)
        pages[i] = malloc(nbytes*nbytes *sizeof(ARRAY_TYPE));


    //New requirement for file 0 !
    for (unsigned i = 0; i < nbytes*nbytes; i++)
            pages[0][i] = i;
        
    
	// Now server is ready to listen and verification

	if ((listen(sockfd, 128)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");

    int client_sock;

	// Function for chatting between client and server
    while( (client_sock = accept(sockfd, (struct sockaddr *)&servaddr, (socklen_t*)&cli)) )
    {
        //printf("server accept the socket\n");
        connection_handler((void*)(intptr_t)client_sock, nbytes, pages);
    }


	// After chatting close the socket
	close(sockfd);
    for (int i = 0; i < npages; i++){
        free(pages[i]);
    }
    free(pages);
}
