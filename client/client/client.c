#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define BUFF_SIZE 4096
#define FILENAME "img1.jpeg"

struct packet {
    char data[4096];
    long data_size;
    int data_seq;
    int isFinished;
};

int main(int argc, const char * argv[]) {
    struct packet pk;
    int client_socket = 0;
    
    struct sockaddr_in server_addr;
    
    char finish_code = 0x1A;

    FILE * fp = NULL;
    size_t file_size;
    int sendSize = 0;
    
    client_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if(-1 == client_socket){
        printf("socket 생성 실패\n");
        exit(1);
    }

    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    fp = fopen(FILENAME, "r");
    if(fp == NULL){
        printf("File Open Error");
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    pk.data_seq = 0;
    pk.isFinished = 0;
    
    while(file_size > 0){
        if(file_size > BUFF_SIZE){
            pk.data_size = BUFF_SIZE;
            pk.isFinished = 0;
        }
        else{
            pk.data_size = file_size;
            pk.isFinished = 1;
        }
        
        fread(pk.data, pk.data_size, 1, fp);
        
        sendto(client_socket, pk.data, pk.data_size, 0, (const struct sockaddr*)&server_addr, sizeof(server_addr));
        printf("%ld byte data (seq: %d) sent.\n", pk.data_size, pk.data_seq);
        
        file_size -= pk.data_size;
        sendSize += pk.data_size;
        pk.data_seq++;
    }
    sendto(client_socket, &finish_code, sizeof(finish_code), 0, (const struct sockaddr*)&server_addr, sizeof(server_addr));
    
    fclose(fp);

    close(client_socket);
    
    
    return 0;
}
