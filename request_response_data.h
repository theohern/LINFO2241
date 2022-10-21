typedef struct client_request_data
{
    unsigned int file_index;
    unsigned int key;
    char buff[264];
}client_request_d;

typedef struct server_response_data
{
   unsigned int errorCode;
   unsigned int file_size;
   char reslt[1029];

}server_response_d;