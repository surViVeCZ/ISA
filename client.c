#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h> 


bool validateIPv4(const char *s)
{
    int len = strlen(s);
    if (len < 7 || len > 15){
        fprintf(stderr,"Wrong address");
        return false;
    }

    char tail[16];
    tail[0] = 0;
    unsigned int d[4];
    int c = sscanf(s, "%3u.%3u.%3u.%3u%s", &d[0], &d[1], &d[2], &d[3], tail);

    if (c != 4 || tail[0]){
        fprintf(stderr,"Wrong address");
        return false;
    }
    for (int i = 0; i < 4; i++){
        if (d[i] > 255){
        fprintf(stderr,"Wrong address");
        return false;
        }
    }

    return true;
}


int main(int argc, char* argv[]){

    int socket_data = socket(AF_INET , SOCK_STREAM , 0); //socket create
    if (socket_data == -1)
    {
        printf("Could not create socket");
    }
    printf("Socket created:\n");

    char* host_address = NULL;
    int port_number = 0;

    if (argc == 1){
        if(strcmp(argv[1],"-h") == 0 ||  strcmp(argv[1],"--help") == 0){
            printf("<option> is on of:\n\n-a <addr> --address <addr>\n\tServer hostname or address to connect to\n-p <port> --port <port>\n\tServer port to connect to\n-h --help\n\trun this help");
            printf("\n\nSupported commands:\nregister <username> <password>\nlogin <username> <password>\nlist\nsend <recipient> <subject> <body>\nfetch <id>\nlogout");
            return 0;
        }else{
            printf("wrong arguments\n");
        }
    }else if(argc == 3){
        if (strcmp(argv[1],"-a") == 0 ||  strcmp(argv[1],"--address") == 0)
        {
            if(validateIPv4(argv[2])){
                host_address = argv[2]; 
            }else{
                return 1;
            }
        }
        else if (strcmp(argv[1],"-p") == 0 ||  strcmp(argv[1],"--port") == 0)
        {
            port_number = atoi(argv[2]); 
        }
    }else if(argc == 5){
        if ((strcmp(argv[1],"-a") == 0 ||  strcmp(argv[1],"--address") == 0) && (strcmp(argv[3],"-p") == 0 ||  strcmp(argv[3],"--port") == 0))
        {
            if(validateIPv4(argv[2])){
                host_address = argv[2]; 
                port_number = atoi(argv[4]); 
            }else{
                return 1;
            }
        }
    }else{
        printf("wrong arguments\n");
        return 1;
    }
    
    printf("address: %s\n", host_address);
    printf("port: %d\n", port_number);
    return 0;
}