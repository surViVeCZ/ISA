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
#define SIZE 1000

char* base64Encoder(char input_str[], int len_str)
{
    // Character set of base64 encoding scheme
    char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
     
    // Resultant string
    char *encoded = (char *) malloc(SIZE * sizeof(char));
    int index, bit_count = 0, padding = 0, val = 0, count = 0, tmp;
    int i, j, k = 0;
     
    // Loop takes 3 characters at a time from
    // input_str and stores it in val
    for (i = 0; i < len_str; i += 3){
            val = 0, count = 0, bit_count = 0;
            for (j = i; j < len_str && j <= i + 2; j++){
                // binary data of input_str is stored in val
                val = val << 8;
                 
                // (A + 0 = A) stores character in val
                val = val | input_str[j];
                 
                // calculates how many time loop
                // ran if "MEN" -> 3 otherwise "ON" -> 2
                count++;
             
            }
 
            bit_count = count * 8;
 
            // calculates how many "=" to append after encoded.
            padding = bit_count % 3;
 
            // extracts all bits from val (6 at a time)
            // and find the value of each block
            while (bit_count != 0){
                // retrieve the value of each block
                if (bit_count >= 6){
                    tmp = bit_count - 6;
                    index = (val >> tmp) & 63;
                    bit_count -= 6;        
                }
                else{
                    tmp = 6 - bit_count;
                    // append zeros to right if bits are less than 6
                    index = (val << tmp) & 63;
                    bit_count = 0;
                }
                encoded[k++] = char_set[index];
            }
    }
 
    // padding is done here
    for (i = 0; i < padding; i++){
        encoded[k++] = '=';
    }
 
    encoded[k] = '\0';
    return encoded;
 
}
 
char* Socket_recv(int socket,char* Rqst,short lenRqst){
    int val;
    char token[1024] = {0};
    char server_msg[1024] = {0};
    val = read( socket , server_msg, 1024);
    //printf("server vrací: %s\n",server_msg);
    
    int pole = 22;
    int k = 0;
    //extrakce tokenu
    while(server_msg[pole] !='\"'){

        token[k] = server_msg[pole];
        pole++;
        k++;
    }
    char* mem = malloc(strlen(token)+1);
    strcpy(mem,token);
    return mem;
}


int Socket_send(int socket,char* Rqst,short lenRqst)
{
    //(register "jmenoo" "aGVzbG9v")(ok "registered user jmenoo")

    
    printf("tohle se posílá: %s\n", Rqst);
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return 0;
    }

    shortRetval = send(socket, Rqst, lenRqst, 0);
    return 0;
}

//vznik tccp spojení
int socket_connect(int socket,int port, char* addr, char* command, char* reg_text, int reg_len){
    
    struct sockaddr_in their_addr;
    if(addr != NULL){
        their_addr.sin_addr.s_addr = inet_addr(addr); //Local Host
    }
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(port); //htons - most significant byte first in network memory

     if (connect(socket, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) < 0) {
        printf("connection failed\n");
        exit(1);
    }else{
        printf("connect\n");
    }

    return 0;
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

    int socket_data = socket(AF_INET , SOCK_STREAM , 0); //socket create(ipv4,tcp,protocol)
    if (socket_data == -1)
    {
        printf("Could not create socket");
    }
    printf("Socket created:\n");

    
    char* host_address = "127.0.0.1"; //localhost as default
    int port_number = 0;
    //enum command{reg=1,login=2,send=3,fetch=4,list=5,logout=6};
    char* command_choice = NULL;
    char* username =NULL;
    char* password = NULL;
    int pass_len = 0;
    char request[200];
    int i = 0;
    int reg_len = 0;
    char* token =  NULL; 
    char* encoded_password = NULL; //base64

    FILE *fp;


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

        if(argc > i){
            username = argv[i+1];  
            password = argv[i+2];
            pass_len = strlen(password);
            if(strcmp(argv[i],"register") == 0){  
                //ověřit zda existuje
                command_choice = "register";
            }else if(strcmp(argv[i],"login") == 0){  
                command_choice = "login";
            }

                encoded_password = base64Encoder(password, pass_len);


                strcpy(request, "(");
                strcat(request, command_choice);
                strcat(request, " \"");
                strcat(request, username);
                strcat(request, "\" \"");
                strcat(request, encoded_password);
                strcat(request, "\")");
                //printf("%s\n", request);
                reg_len = strlen(request);
           
            //(login "jmenooooo" "aGVzbG8=")(ok "user logged in" "am1lbm9vb29vMTYzMzYxMjU2NzA1Ny4wMzI3")
        }
    }  
    
    socket_connect(socket_data, port_number,host_address, command_choice, request, reg_len);
    if(command_choice == "register" || command_choice == "login"){ 
        Socket_send(socket_data, request, reg_len);
        if(command_choice == "login"){
            token = Socket_recv(socket_data, request, reg_len);
            printf("token:%s\n", token);
        }
    }
    fp = fopen("login-token.txt","w+");
    fprintf(fp,"%s",token);
    // printf("name: %s\n", username);
    // printf("pass: %s\n", password);

    // printf("address: %s\n", host_address);
    // printf("port: %d\n", port_number);

    fclose(fp);
    return 0;
}