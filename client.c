#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h> 
#define SIZE 1000
#define BUFFER 262143 //lze přijmout až 26kB dat

/**
 * @see https://www.geeksforgeeks.org/encode-ascii-string-base-64-format/
 * @author https://auth.geeksforgeeks.org/user/ashug716716716 
 * @author https://auth.geeksforgeeks.org/user/shivanisinghss2110 
 * @author https://auth.geeksforgeeks.org/user/prajalpatidar01
 * 
*/char* base64Encoder(char input_str[], int len_str)
{
    // Character set of base64 encoding scheme
    char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *encoded = (char *) malloc(SIZE * sizeof(char));
    int index, bit_count = 0;
    int padding = 0;
    int val = 0;
    int count = 0;
    int tmp;
    int i, j, k = 0;
     
    for (i = 0; i < len_str; i += 3){
            val = 0;
            count = 0;
            bit_count = 0;
            for (j = i; j < len_str && j <= i + 2; j++){
                // binary data of input_str is stored in val
                val = val << 8;
                val = val | input_str[j];
                count++;
            }
 
            bit_count = count * 8;
            padding = bit_count % 3;
 
            // extracts all bits from val (6 at a time)
            while (bit_count != 0){
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
    // padding
    for (i = 0; i < padding; i++){
        encoded[k++] = '=';
    }
 
    encoded[k] = '\0';
    return encoded;
}

/**
 * @see https://gist.github.com/jirihnidek/bf7a2363e480491da72301b228b35d5d
 * @author Jiri Hnidek
 * */
//funkce vrací 4 pokud se jedná o ipv4, nebo 6 v případě ipv6
int ipv4_or_ipv6(char* host_address){
    struct addrinfo hints, *res, *result;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;
    int proto = 4;

     errcode = getaddrinfo (host_address, NULL, &hints, &result);
    if (errcode != 0){
      fprintf(stderr,"Wrong address\n");
      return 1;
    }
    res = result;
    while (res)
        {
        inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);
        switch (res->ai_family){
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
        }

    inet_ntop (res->ai_family, ptr, addrstr, 100);
    if(res->ai_family == PF_INET6){
        proto = 6;
    }else{
        proto = 4;
    }
    return proto;
    }
    
}

//funkce vrací odpověď serveru
char* server_recv(int socket,char* Rqst,short lenRqst, char* host_address){
    int val;
    char token[BUFFER] = {0};
    char server_msg[BUFFER] = {0};
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 5;  
    tv.tv_usec = 0;

    if(setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        fprintf(stderr,"Time Out\n");
        exit(1);
    }


    val = read( socket , server_msg, BUFFER);
    if(val < 0) {
        fprintf(stderr,"Time Out\n");
        exit(1);
    }
    //printf("server vrací: %s\n",server_msg);

    
    int index = 0;
    int k = 0;
    //extrakce tokenu
    while(server_msg[index] != '\0'){

        token[k] = server_msg[index];
        index++;
        k++;
    }
    token[k] = '\0';
    //strncpy(token, server_msg, val);
    //token[val] = '\0';
    char* mem = malloc(strlen(token)+1);
    if(mem == NULL){
        fprintf(stderr,"Bad alloc\n");
        exit(1);
    }
    strcpy(mem,token);
    return mem;
}
 
 char* get_token(FILE* fp){
    char* token = NULL;
    char buffer[BUFFER];

    fp = fopen("login-token.txt","r");
    token = fgets(buffer,50,fp);
    fclose(fp);
    return token;
}

//posílání informací klient -> server 
int Socket_send(int socket,char* Rqst,short lenRqst){
    //(register "jmenoo" "aGVzbG9v")(ok "registered user jmenoo")
 
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 5;  
    tv.tv_usec = 0;
    //nastavení timeoutu
    if(setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return 1;
    }

    shortRetval = send(socket, Rqst, lenRqst, 0);
    return 0;
}

//vytvoření tcp spojení
int socket_connect(int socket,int port, char* addr, char* command, char* reg_text, int reg_len, int protocol){
    
    struct sockaddr_in their_addr;
    if(addr != NULL){
        their_addr.sin_addr.s_addr = inet_addr(addr); //Local Host
    }


    if(protocol == 4){
        their_addr.sin_family = AF_INET;
    }else{
        their_addr.sin_family = AF_INET6 ;
    }

    their_addr.sin_port = htons(port); //htons - most significant byte first in network memory

    int shortRetval = -1;
    struct timeval tv;

    tv.tv_sec = 5;  
    tv.tv_usec = 0;

    
    if(setsockopt(socket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return 1;
    }
    //TCP spojení
    if (connect(socket, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr,"tcp-connect: connection failed\n hostname: %s\n port number: %d\n system error: Connection refused; errno=111\n", addr, port);
        exit(1);
    }
    return 0;
}


int main(int argc, char* argv[]){

    int port_number = 0;
    int pass_len = 0;
    int reg_len = 0;
    
    char* command_choice = NULL;
    char* username = NULL;
    char* password = NULL;
    char* token = NULL;
    char* server_answer = NULL;
    char* encoded_password = NULL; //base64
    char* msg_subject = NULL;
    char* msg_body = NULL; 
    char* host_address = NULL;
    
    //1024*1024 = 1MB of data
    char request[BUFFER];
    char msg_req[BUFFER];
    char buff[BUFFER];
    char host[255];
    char *local_ip = "127.0.0.1"; //výchozí hodnota adresy

    bool port_set = false;
    bool address_set = false;
    
    int i = 0;
    char* fetch_id;
    FILE *fp;

    if (argc == 1){
        fprintf(stderr, "client: expects <command> [<args>] ... on the command line, given 0 arguments\n");
        return 0;
    }

    //zpracování argumentů -a -p -h 
    for(i = 1; i < argc; i++){
        //nápověda
        if(strcmp(argv[i],"-h") == 0 ||  strcmp(argv[i],"--help") == 0){
            printf("<option> is on of:\n\n-a <addr> --address <addr>\n\tServer hostname or address to connect to\n-p <port> --port <port>\n\tServer port to connect to\n-h --help\n\trun this help");
            printf("\n\nSupported commands:\nregister <username> <password>\nlogin <username> <password>\nlist\nsend <recipient> <subject> <body>\nfetch <id>\nlogout");
            return 0;
        //adresa
        }else if(strcmp(argv[i],"-a") == 0 ||  strcmp(argv[i],"--address") == 0){
            address_set = true;
            if(argv[i+1] != NULL){
                if(strcmp(argv[i+1], "localhost") == 0){
                    host_address = local_ip;
                    i++;
                }else{
                    host_address = argv[i+1]; 
                    i++;
                }
            }
        //port
        }else if(strcmp(argv[i],"-p") == 0 ||  strcmp(argv[i],"--port") == 0){
            port_set = true;
            if(argv[i+1] != NULL){ 
                port_number = atoi(argv[i+1]); 
                if(port_number <= 0){ 
                    fprintf(stderr, "Port number is not a string\n");
                    return 1;
                }
                i++;
            }
        }else{
            break;
        }
    }

    //pokud nebyla adresa nebo port zadány, nastavuji výchozí hodnotu
    if(address_set == false){
        host_address = local_ip; //výchozí ip adresa 127.0.0.1
    }
    if(port_set == false){
        port_number = 32323; //výchozí číslo portu
    }
    
    int protocol;
        //nutno zjisit o jaký protokol se jedná
    protocol = ipv4_or_ipv6(host_address);
     
    int socket_data;

    //vytvoření socketu na zíkladě hodnoty protocol, která značí jestli se jedná o ipv4 nebo ipv6
    if(protocol == 4){
        socket_data = socket(AF_INET , SOCK_STREAM , 0); //socket pro ipv4
    }else if(protocol == 6){
        socket_data = socket(AF_INET6 , SOCK_STREAM , 0); //socket pro ipv6
    }else{
        fprintf(stderr,"Could not create socket\n");
        exit(1);
    }

    if (socket_data == -1)
    {
        fprintf(stderr,"Could not create socket\n");
        exit(1);
    }

    //zpracování všech commandů
    if(argc > i){   
        if(strcmp(argv[i],"register") == 0){  
            //ověřit zda existuje
            command_choice = "register";
            //(argv[i+3] == NULL) - za heslem už nesmí být argumenty
            if((argv[i+1] != NULL) && (argv[i+2] != NULL) && (argv[i+3] == NULL)){
                username = argv[i+1];  
                password = argv[i+2];
                pass_len = strlen(password);
                encoded_password = base64Encoder(password, pass_len);
            }else{
                fprintf(stderr,"register <username> <password>\n");
                return 0;
            }
        }else if(strcmp(argv[i],"login") == 0){  
            if((argv[i+1] != NULL) && (argv[i+2] != NULL) && (argv[i+3] == NULL)){
                command_choice = "login";
                username = argv[i+1];  
                password = argv[i+2];
                pass_len = strlen(password);
            }else{
                fprintf(stderr,"login <username> <password>\n");
                return 0;
            }
        }else if(strcmp(argv[i],"send") == 0){  
            command_choice = "send";
             if((argv[i+1] != NULL) && (argv[i+2] != NULL) && (argv[i+3] != NULL) && (argv[i+4] == NULL)){
                username = argv[i+1];  
                msg_subject = argv[i+2];
                msg_body = argv[i+3];
                password = "password";
                //ošetření excape sekvencí
                if((strcmp(msg_body,"\\\\") == 0)){
                    printf("msg_body: %s\n", msg_body);
                }else if((strcmp(msg_subject,"\\\\") == 0)){
                    msg_subject = "\\";
                }else if((strcmp(msg_body,"\n") == 0)){
                    msg_body = "n";
                }else if((strcmp(msg_subject,"\\\n") == 0)){
                    msg_subject = "n";
                }
            }else{
                fprintf(stderr,"send <recipient> <subject> <body>\n");
                return 0;
            }
        }else if(strcmp(argv[i],"list") == 0){  
            command_choice = "list";
            username = " ";
            password = " ";
        }else if(strcmp(argv[i],"fetch") == 0){  
            command_choice = "fetch";
            username = " ";
            password = " ";
            if(argv[i+1] != NULL){
                int id = atoi(argv[i+1]);
                if(id != 0){
                   fetch_id = argv[i+1]; 
                }else{
                    fprintf(stderr,"ERROR: id %s is not a number\n",argv[i+1]);
                    return 1;
                }
                username = " ";
                password = " ";
            }else{
                fprintf(stderr,"fetch <id>\n");
                return 0;
            }
            
        }else if(strcmp(argv[i],"logout") == 0){  
            command_choice = "logout";
            username = " ";
            password = " ";
        }else{ 
            fprintf(stderr,"Unknown command\n");
            return 0;
        }

        //zakódované heslo pomocí hashovací funkce base64
        encoded_password = base64Encoder(password, pass_len);

        //vytvoření zprávy, kterou pošlu serveru
        strcpy(request, "(");
        strcat(request, command_choice);
        strcat(request, " \""); 
        strcat(request, username);
        strcat(request, "\" \"");
        strcat(request, encoded_password);
        strcat(request, "\")");
        //printf("%s\n", request);
        reg_len = strlen(request);
    

        if(strcmp(argv[i],"send") != 0){
            socket_connect(socket_data, port_number,host_address, command_choice, request, reg_len, protocol); //odesílání zpáv, jíný connect
        }
        
        }
        //nebyl zadán žádný příkaz
        if (command_choice == NULL){ 
            printf("Client expects <command> on the line, 0 arguments given\n");
            return 0;
        }


        if(command_choice == "register" || command_choice == "login"){
            
            Socket_send(socket_data, request, reg_len);

            //tisknutí odpovědi serveru
            server_answer = server_recv(socket_data, request, reg_len, host_address);
            //snaha o přihlášení neregistrovaného uživatele, nebo registrace podruhé
            if(strcmp(server_answer,"(err \"unknown user\")") == 0){ 
                fprintf(stderr,"ERROR: unknown user\n");
                return  1;
            }else if(strcmp(server_answer,"(err \"user already registered\")") == 0){
                fprintf(stderr,"ERROR: user already registered\n");
                return  1;
            }
      
            if(command_choice == "register"){
                printf("SUCCESS: registered user %s\n", username);
            }else{
                printf("SUCCESS: user logged in\n");
            }
            //extrakce login-tokenu
            int index_start = 22;
            int k = 0;
            char token[1024] = {0};
            while(server_answer[index_start] !='\"'){
                token[k] = server_answer[index_start];
                index_start++;
                k++;
            }
            //uložení tokenu do souboru login-token.txt
            fp = fopen("login-token.txt","w+");
                fprintf(fp,"%s",token);
                fclose(fp);
        } else if(command_choice == "send"){
            token = get_token(fp);
            int msg_req_len = 0;
            int protocol;
        
            //vytvoření requestu, který pošleme na server
            strcpy(msg_req, "(");
            strcat(msg_req, command_choice);
            strcat(msg_req, " \"");
            strcat(msg_req, token); 
            strcat(msg_req, "\" \"");
            strcat(msg_req, username);
            strcat(msg_req, "\" \"");
            strcat(msg_req, msg_subject);
            strcat(msg_req, "\" \"");
            strcat(msg_req, msg_body);
            strcat(msg_req, "\")");
            msg_req_len = strlen(msg_req);

            //zistím o jaký protokol se jedná
            protocol = ipv4_or_ipv6(host_address);
            socket_connect(socket_data, port_number,host_address, command_choice, msg_req, msg_req_len, protocol);
            Socket_send(socket_data, msg_req, msg_req_len);

            //odpověď serveru
            server_answer = server_recv(socket_data, request, msg_req_len, host_address);
            if(strcmp(server_answer,"(err \"unknown recipient\")") == 0){ 
                fprintf(stderr,"ERROR: unknown recipient\n");
                return  1;
            }

            printf("SUCCESS: message sent\n");
        }else if(command_choice == "list"){

            //list nemá argumenty
            if(argv[i+1] != NULL){
                fprintf(stderr, "list\n");
                return 1;
            }
            token = get_token(fp);
            int list_len = 0;

            //zpráva kterou posílám serveru
            strcpy(msg_req, "(");
            strcat(msg_req, command_choice);
            strcat(msg_req, " \"");
            strcat(msg_req, token);
            strcat(msg_req, "\")");
            list_len = strlen(msg_req);
            Socket_send(socket_data, msg_req, list_len);


            char* delim = "\"";
            char* object;
            char* from;
            char *informations[255];

            //odpověď serveru
            server_answer = server_recv(socket_data, request, list_len, host_address);
                 
            //naparsování odpovědi
            char* info = strtok(server_answer, delim);

            int i = 0;
            while(info != NULL){
                informations[i] = info;
                info = strtok(NULL, delim);
                i++;
            }
            
            printf("SUCCESS:\n");

            //parsování odpovědi serveru
            int id = 1;
            for(int index = 1; informations[index] != NULL; index+=4){
                from = informations[index]; //posun o 4 - parusuju podle ""
                object = informations[index+2]; //posun o 4
                printf("%d:\n   From: %s\n   Subject: %s\n", id, from, object);
                id++;
            }
            

        }else if(command_choice == "fetch"){
            token = get_token(fp);
            int list_len = 0;

            //vytvoření requestu, který pošleme na server
            strcpy(msg_req, "(");
            strcat(msg_req, command_choice);
            strcat(msg_req, " \"");
            strcat(msg_req, token);
            strcat(msg_req, "\" ");
            strcat(msg_req, fetch_id);
            strcat(msg_req, ")");
            list_len = strlen(msg_req);
            Socket_send(socket_data, msg_req, list_len);

            //odpověď serveru
            server_answer = server_recv(socket_data, request, list_len,host_address);
            char* delim = "\"";

            char* from;
            char* object;
            char* message;
            char *informations[255] = {0};

            //parsování odpovědi serveru
            char* info = strtok(server_answer, delim);

            int i = 0;
            while(info != NULL){
                informations[i] = info;
                info = strtok(NULL, delim);
                i++;
            }
            
            //jednotlivé části výstupu fetch
            from = informations[1];
            object = informations[3];
            message = informations[5];

            //pokud si uživatel vybere id, které neexistuje, vypíšeme chybu
            if((object == NULL)  || (message == NULL)){
                fprintf(stderr, "%s\n", from);
                return 1;
            };
            printf("SUCCESS:\n\nFrom: %s\nSubject: %s\n\n%s", from, object, message);


        }else if(command_choice == "logout"){   
            //logout nemá argumenty
            if(argv[i+1] != NULL){
                fprintf(stderr, "logout\n");
                return 1;
            }

            token = get_token(fp);
            int list_len = 0;

            //vytvoření requestu, který pošleme na server
            strcpy(msg_req, "(");
            strcat(msg_req, command_choice);
            strcat(msg_req, " \"");
            strcat(msg_req, token);
            strcat(msg_req, "\")");
            list_len = strlen(msg_req);
            Socket_send(socket_data, msg_req, list_len);

            //přijatá odpověď
            server_answer = server_recv(socket_data, request, list_len, host_address);
            printf("SUCCESS: logged out\n");
        }
    return 0;
}