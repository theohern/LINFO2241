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
#define PORT 8080
#define SA struct sockaddr
int npages = 1000;

// Function designed for chat between client and server.
void connection_handler(void *socket_desc,int nbytes, int32_t **pages)
{
    printf("connction_handler begin\n");
    int fileid;
    int keysz;
    int sockfd = (int)(intptr_t)socket_desc;

    int tread = recv(sockfd, &fileid, 4, 0);
    //fileid = 5;
    tread = recv(sockfd, &keysz, 4, 0);
    //Network byte order
    //keysz = ntohl(keysz);
    keysz = 128;
    fileid = 123;

    printf("keysz=%d, fileid=%d\n", keysz, fileid);
    int32_t key[keysz*keysz];
    unsigned tot = keysz*keysz * sizeof(int32_t);

    unsigned done = 0;
    while (done < tot) {
        tread = recv(sockfd, key, tot- done, 0);
        done += tread;
    }

    int nr = nbytes / keysz;
    int32_t* file = pages[fileid % npages];
    int32_t* crypted = malloc(nbytes*nbytes * sizeof(int32_t));
    //Compute sub-matrices
    for (int i = 0; i < nr ; i ++) {
        int vstart = i * keysz;
        for (int j = 0; j < nr; j++) {
            int hstart = j * keysz;

            //Do the sub-matrix multiplication
            for (int ln = 0; ln < keysz; ln++) {

                int aline = (vstart + ln) * nbytes + hstart;
                for (int col = 0; col < keysz; col++) {

                    int tot = 0;
                    for (int k = 0; k < keysz; k++) {
                        int vline = (vstart + k) * nbytes + hstart;
                        tot += key[ln * keysz + k] * file[vline + col];
                    }
                    crypted[aline + col] = tot;

                }
            }
        }
    }

    // for (int i = 0; i < nr ; i ++) {
    //     int vstart = i * keysz;
    //     for (int j = 0; j < nr; j++) {
    //         int hstart = j * keysz;

    //         //Do the sub-matrix multiplication
    //         for (int ln = 0; ln < keysz; ln++) {

    //             int aline = (vstart + ln) * nbytes + hstart;
    //             for (int k = 0; k < keysz; k++) {

    //                 int a = key[ln * keysz + k];
    //                 int vline = (vstart + k) * nbytes + hstart;
    //                 for (int col = 0; col < keysz; col+=8) {
    //                     crypted[aline + col] +=  a * file[vline + col];
    //                     crypted[aline + col +1] +=  a * file[vline + col +1];
    //                     crypted[aline + col +2] +=  a * file[vline + col +2];
    //                     crypted[aline + col +3] +=  a * file[vline + col +3];
    //                     crypted[aline + col +4] +=  a * file[vline + col +4];
    //                     crypted[aline + col +5] +=  a * file[vline + col +5];
    //                     crypted[aline + col +6] +=  a * file[vline + col +6];
    //                     crypted[aline + col +7] +=  a * file[vline + col +7];
    //                 }

    //             }
    //         }
    //     }
    // }





    int err = 0;
    send(sockfd, &err, 1,MSG_NOSIGNAL );
    unsigned sz = htonl(nbytes*nbytes * sizeof(int32_t));
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    send(sockfd, crypted, nbytes*nbytes * sizeof(int32_t),MSG_NOSIGNAL );
    printf("fin du send\n");
    free(crypted);


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

    int32_t **pages = malloc(sizeof(int32_t*) * npages);
    for (int i = 0 ; i < npages; i++)
        pages[i] = malloc(nbytes*nbytes *sizeof(int32_t));


    printf("pages en mémoire\n");
    //New requirement for file 0 !
    for (unsigned i = 0; i < nbytes*nbytes; i++)
            pages[0][i] = i;
        
    
    printf("pages ok\n");
	// Now server is ready to listen and verification

	if ((listen(sockfd, 128)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	//len = sizeof(cli);

	// Accept the data packet from client and verification
	// connfd = accept(sockfd, (struct sockaddr *)&servaddr, &cli);
	// if (connfd < 0) {
	// 	printf("server accept failed...\n");
	// 	exit(0);
	// }
	// else
	// 	printf("server accept the client...\n");

    int client_sock;

	// Function for chatting between client and server
    while( (client_sock = accept(sockfd, (struct sockaddr *)&servaddr, (socklen_t*)&cli)) )
    {
        printf("server accept the socket\n");
        connection_handler((void*)(intptr_t)client_sock, nbytes, pages);
    }


	// After chatting close the socket
	close(sockfd);
    for (int i = 0; i < npages; i++){
        free(pages[i]);
    }
    free(pages);
}
