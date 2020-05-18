#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#define BUFF_SIZE 4096
#define FILENAME "img1.jpeg"

struct packet {
    char data[4096];
    long data_size;
    int data_seq;
    int isFinished;
};

int isFirst = 1;

void packet(struct packet pk){
    FILE *fp = NULL;
    long int offset;
    
    if(isFirst == 1){
        fp = fopen(FILENAME, "w+");
        if(fp == NULL){
            printf("File Open Error");
            exit(1);
        }
        isFirst = 0;
    }else{
        fp = fopen(FILENAME, "a");
        if(fp == NULL){
            printf("File Open Error");
            exit(1);
        }
    }
    
    offset = BUFF_SIZE * pk.data_seq;
    
    fseek(fp, offset, SEEK_SET);
    
    fwrite(pk.data, pk.data_size, 1, fp);
    fclose(fp);
}

int main(int argc, const char * argv[]) {
    struct packet pk;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    unsigned int client_addr_size = 0;

    int server_socket = 0;
    
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == server_socket){
        printf("server socket 생성 실패\n");
        exit(1);
    }
    char finish_code = 0x1A;
    int port_num = 4000;
        
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    
    if(-1 == bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        printf("bind() 실행 에러\n");
        exit(1);
    }
            
    pk.data_seq = 0;
    pk.isFinished = 0;

    while(1){
        client_addr_size = sizeof(client_addr);
        if(-1 == (pk.data_size = recvfrom(server_socket, pk.data, BUFF_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_size))){
            exit(1);
        }
        
        if(*(pk.data) == finish_code)
            break;
        
        printf("%ld byte data (seq: %d) received.\n", pk.data_size, pk.data_seq);
        
        packet(pk);
        
        pk.data_seq++;
    }

    return 0;
}
