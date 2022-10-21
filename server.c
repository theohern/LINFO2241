#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "request_response_data.h"

#define MAX_CLIENT_SUPPORTED 100
#define SERVER_PORT 2000
#define MAX 20

client_request_d client_data;
server_response_d server_result;
char data_buffer[1048584];

int controlfd_set[64];

static void
intitiaze_monitor_fd_set()
{

    int i = 0;
    for (; i < MAX_CLIENT_SUPPORTED; i++)
        controlfd_set[i] = -1;
}

static void
add_to_controlfd_set(int skt_fd)
{

    int i = 0;
    for (; i < MAX_CLIENT_SUPPORTED; i++)
    {

        if (controlfd_set[i] != -1)
            continue;
        controlfd_set[i] = skt_fd;
        break;
    }
}

static void
remove_from_controlfd_set(int skt_fd)
{

    int i = 0;
    for (; i < MAX_CLIENT_SUPPORTED; i++)
    {

        if (controlfd_set[i] != skt_fd)
            continue;

        controlfd_set[i] = -1;
        break;
    }
}

static void re_init_readfds(fd_set *fd_set_ptr)
{

    FD_ZERO(fd_set_ptr);

    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (controlfd_set[i] != -1)
        {
            FD_SET(controlfd_set[i], fd_set_ptr);
        }
    }
}

static int get_max_fd()
{

    int i = 0;
    int max = -1;

    for (; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (controlfd_set[i] > max)
            max = controlfd_set[i];
    }

    return max;
}

//multiplication of 2 matrix
void matrix_mult(int size, char** key, char** file, int** encrypt_file, int x, int y){
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            for (int k = 0; k < size; k++){
                encrypt_file[i+x][j+y] += (int)key[i][k] * (int)file[k+x][j+y];
            }
        }
    }
}

//ecnryption of the file with the key
void encryption(int size, int N, char** key, char** file, int** encrypt_file){
    int n = (size/N);
    for (int i = 0; i < n; i ++){
        for (int j = 0; j < n; j++){
            matrix_mult(N, key, file, encrypt_file, i*n, j*n);
        }
         //server_result.reslt[i] = encrypt_file;
    }
   
}

// print a matrix
void print(int n, int** mat){
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            printf("%d ", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void printchar(int n, char** mat){
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            printf("%d ", (int)(mat[i][j]));
        }
        printf("\n");
    }
    printf("\n");
}
   
void tcp_server_communication(char** files, int n_clients, int port, int s)
{   
    
    /*Initialize master socket file descriptor */
    int master_sock_tcp_fd = 0,
        sent_recv_data = 0,
        addr_len = 0,
        opt = 1;

    int comm_socket_fd = 0;
    fd_set readfds;

    /*store the server and client info*/
    struct sockaddr_in server_addr,
        client_addr;

    intitiaze_monitor_fd_set();

    /*tcp master socket creation*/
    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("socket creation failed\n");
        exit(1);
    }

    /*server Information*/
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    addr_len = sizeof(struct sockaddr);

    /* Bind the server*/

    if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("socket bind failed\n");
        return;
    }

    /*S To maintain the queue of max length incoming client -----?*/

    if (listen(master_sock_tcp_fd, n_clients) < 0)
    {
        printf("listen failed\n");
        return;
    }

    /*Add master socket to controlfds*/
    add_to_controlfd_set(master_sock_tcp_fd);

    while (1)
    {

        /* Initialize the file descriptor set*/
        re_init_readfds(&readfds);
        printf("blocked on  and select System call...\n");

        /* Wait for client connection*/
        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        /*If Data arrives on master socket FD*/
        if (FD_ISSET(master_sock_tcp_fd, &readfds))
        {
            printf("New connection recieved recvd, accept the connection.\n");

            /* accept() returns a new temporary file desriptor(fd) */
            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
            if (comm_socket_fd < 0)
            {

                printf("accept error : errno = %d\n", errno);
                exit(0);
            }

            add_to_controlfd_set(comm_socket_fd);
            printf("Connection accepted from client : %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        /* Data srrives on from other client FD*/
        else
        {

            int i = 0, comm_socket_fd = -1;
            for (; i < MAX_CLIENT_SUPPORTED; i++)
            {

                /*Find the clinet FD on which Data has arrived*/
                if (FD_ISSET(controlfd_set[i], &readfds))
                {

                    comm_socket_fd = controlfd_set[i];

                    memset(data_buffer, 0, sizeof(data_buffer));
                    sent_recv_data= recvfrom(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0, (struct sockaddr *)&client_addr, &addr_len);

                    printf("Server recvd %d bytes from client %s:%u\n", sent_recv_data,
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    if (sent_recv_data == 0)
                    {
                        /*If server recvs empty msg from client, server may close the connection and wait */
                        close(comm_socket_fd);
                        remove_from_controlfd_set(comm_socket_fd);
                        break;
                    }
    
                    // The code  merg
                    client_request_d *client_data = (client_request_d *)data_buffer;

                    /* If the client sends a special msg to server, then server close the client connection */
                    if (client_data->buff == 0)
                    {

                        close(comm_socket_fd);
                        remove_from_controlfd_set(comm_socket_fd);
                        printf("Server closes connection with client : %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        break;
                    }

                    //server_response_d result;                    

                    //printf("Request from client %s\n: ", client_data->buff);

                    //incription
                      char buf[8+(s*s)];
                      strcpy(buf, client_data->buff);
                      //"362500021234";


                    char* f_n = malloc(sizeof(char)*4);
                    char* s_k = malloc(sizeof(char)*4);

                    for (int i = 0; i < 4; i++){
                        f_n[i] = buf[i];
                        s_k[i] = buf[i+4];
                    }
                    int file_name = atoi(f_n);
                    int size_key = atoi(s_k);
                    
                    char* k = malloc(sizeof(int)*size_key*size_key);
                    int* e = malloc(sizeof(int)*s*s);

                    char** f = malloc(sizeof(char*)*s);
                    for (int i = 0; i < s; i ++){
                        f[i] = &(files[file_name][i*s]);
                    }


                    char** key = malloc(sizeof(int*)*size_key);
                    int** encrypt_file = malloc(sizeof(int*)*s);
                    for (int i = 0; i < s; i++){
                        encrypt_file[i] = &e[i*s];
                    }

                    for (int i = 0; i < size_key; i++){
                        key[i] = &k[i*size_key];
                    }

                    
                   

                    for (int i = 0; i < size_key*size_key; i++){
                        k[i] = buf[i+8]-48;
                    }
                    //printf("ok\n");
                    //printf("buffer : %s \n", buf);                    
                    //printf("file_name : %d   size_key : %d\n\n", file_name, size_key);

                    //printchar(size_key, key);

                    //printchar(s, f);

                    encryption(s, size_key, key, f, encrypt_file);

                    server_result.reslt[0] = '0';
                    server_result.reslt[1] = '0';
                    server_result.reslt[2] = '0';
                    server_result.reslt[3] = '0';
                    server_result.reslt[4] = '0';
                    

                    for (int i = 0; i < 16; i++){
                        server_result.reslt[i+5] = e[i];
                    }

                    //printf("Encription\n");

                    //print(s, encrypt_file);

                    //strcpy(server_result.reslt,enc);
                   
                    /* Server replying back to client */
                    sent_recv_data = sendto(comm_socket_fd, (char *)&server_result, sizeof(server_response_d), 0,
                                             (struct sockaddr *)&client_addr, sizeof(struct sockaddr));

                    //printf("Server sent %d bytes in reply to client\n", sent_recv_data);
                    

                    
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    int opt;
    int port = 2000;
    int nthread = 4;
    int size = 4;


    while((opt = getopt(argc, argv, "j:s:p:?")) != -1){
        switch(opt){
            case 'j':
                nthread = atoi(optarg);
                break;
            case 's':
                size = atoi(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default :
                printf("something goes wrong !");
        }
    }
    //printf("size of files : %d\n", size);
    char** files = malloc(sizeof(char*)*1000);  
    for (int i = 0; i < 1000; i++){
        files[i] = malloc(sizeof(char)*size*size);
        for (int j = 0; j < size*size; j++){
            files[i][j] = rand()%92+33;
        }
    }

    tcp_server_communication(files, nthread, port, size);
    return 0;
}