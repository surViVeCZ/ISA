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

//vznik tccp spojení
int socket_connect(int socket,int port, char* addr){ 
    struct sockaddr_in their_addr;
    int retval = -1;
    their_addr.sin_addr.s_addr = inet_addr(addr); //Local Host
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(port); //htons - most significant byte first in network memory
    retval = connect(socket,(struct sockaddr *)&their_addr,sizeof(struct sockaddr_in));
    return retval;

     if (connect(socket, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }
}

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
    //enum command{reg=1,login=2,send=3,fetch=4,list=5,logout=6};
    char* command_choice = NULL;
    char* username =NULL;
    char* password = NULL;
    int i = 0;


    if (argc == 1){
        printf("No arguments");
        return 0;
    }
    else{
        for(i = 1; i < argc; i++){
            if(strcmp(argv[i],"-h") == 0 ||  strcmp(argv[i],"--help") == 0){
                printf("<option> is on of:\n\n-a <addr> --address <addr>\n\tServer hostname or address to connect to\n-p <port> --port <port>\n\tServer port to connect to\n-h --help\n\trun this help");
                printf("\n\nSupported commands:\nregister <username> <password>\nlogin <username> <password>\nlist\nsend <recipient> <subject> <body>\nfetch <id>\nlogout");
                return 0;
            }else if(strcmp(argv[i],"-a") == 0 ||  strcmp(argv[i],"--address") == 0){
                if(validateIPv4(argv[i+1])){
                host_address = argv[i+1]; 
                i++;
                }else{
                    return 1;
                }
            }else if(strcmp(argv[i],"-p") == 0 ||  strcmp(argv[i],"--port") == 0){
                port_number = atoi(argv[i+1]); 
                i++;
            }else{
                break;
            }
        }
        if(strcmp(argv[i],"register") == 0){    
            //ověřit zda existuje
            username = argv[i+1];  
            password = argv[i+2];  
        }
    }  
 
    
printf("name: %s\n", username);
    printf("pass: %s\n", password);

    printf("address: %s\n", host_address);
    printf("port: %d\n", port_number);
    return 0;
}