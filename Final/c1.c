#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>

#define SIZE_B 1024
#define prt_s 56789
#define tmout 2 // retransmission tmout duration

bool reject;



typedef struct packet{
    int size;
    int offset;// ye seq no bol ke print karwana hai
    int type;//0 for data packet 1 from ack packet
    char data[SIZE_B];
    int seq_no;//ye apna scam hai 4 state ke liye fsm me kyunki jarorat to nahi hai
} pkt;

void error_exit(char *e1){
    perror(e1);
    exit(EXIT_FAILURE);
}


int main() {
    // create a TCP socket
    int skt_crt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (skt_crt == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set up the server address and port
    struct sockaddr_in s_ad;
    s_ad.sin_family = AF_INET;
    s_ad.sin_addr.s_addr = inet_addr("127.0.0.1");
    s_ad.sin_port = htons(prt_s);

    // connect to the server
    if (connect(skt_crt, (struct sockaddr*)&s_ad, sizeof(s_ad)) != 0) {
        perror("Connection with the server failed");
        exit(EXIT_FAILURE);
    }

    
    FILE* fp = fopen("name.txt", "r");
    if (fp == NULL) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    
    struct packet pk_s, ak_rcv;

    int state_symbol = 0;
    int flg_st = 0;
    // int seq_no = 0;
    while (!feof(fp)) {
        
        switch(state_symbol)
        {
            case 0: 
            {
                char id_file_id[256];
                if (fscanf(fp, "%[^,.],", id_file_id) != 1) {
                    flg_st = 1;
                    break; // end of file
                }
                printf("%s\n", id_file_id);
                memset(&pk_s,0,sizeof(pk_s));
                pk_s.type = 0; // data packet
                pk_s.seq_no = 0; // sequence number
                pk_s.size = strlen(id_file_id);
                strcpy(pk_s.data, id_file_id);
  
                int socket_fd = send(skt_crt , &pk_s, sizeof(pkt), 0);
                if ( socket_fd < 0)
                {
                    error_exit("sendto()");
                }
                printf("pkt 1 -> name: %s, seq_no: %d\n", pk_s.data, pk_s.seq_no);
                state_symbol = 1;
            }
            break; 

            case 1:
            {//waiting for ACK 0

                fd_set rcvSet;
                int n;
                
                struct timeval tv;

                FD_ZERO(&rcvSet);
                FD_SET(skt_crt, &rcvSet);

                tv.tv_sec = tmout;
                tv.tv_usec = 0;

                if((n = select(skt_crt + 1, &rcvSet, NULL, NULL, &tv) ) < 0){
                    error_exit("error on select");
                }
                
                if(n == 0) {     // tmout expired, send packet again and remain in this state
                    printf("tmout occured\n");
                    if (send(skt_crt , &pk_s, sizeof(pk_s), 0)== -1){
                    error_exit("sendto()");
                    }
                    break;
                }
                
                // socket is readable => ack arrived
                if (recv(skt_crt , &ak_rcv, sizeof(pkt), 0) == -1){
                    error_exit("recv()");
                }               
                printf("Ack1 received\n");
                if(ak_rcv.seq_no == 0){  // if ACK0 arrived, change the state                        
                    state_symbol = 2;                    
                }
                else{
                    break;
                }

                printf("Received ack seq. no. %d\n\n",ak_rcv.seq_no);
            }
            break;

            case 2:
            {
                char id_file_id[256];
                if (fscanf(fp, "%[^,.],", id_file_id) != 1) {
                    flg_st = 1;
                    break; // end of file
                }
                printf("%s\n", id_file_id);
                memset(&pk_s,0,sizeof(pk_s));
                // create a data packet for the student name
                pk_s.type = 0; // data packet
                pk_s.size = strlen(id_file_id);
                memcpy(pk_s.data, id_file_id, pk_s.size);

                pk_s.seq_no = 1;     
                if (send(skt_crt , &pk_s, sizeof(pkt), 0)== -1){
                    error_exit("sendto()");
                }
                state_symbol = 3; 
            }
            break;                    

            case 3: //waiting for ACK 1
            {

                fd_set rcvSet;
                int n;
                
                struct timeval tv;

                FD_ZERO(&rcvSet);
                FD_SET(skt_crt, &rcvSet);

                tv.tv_sec = tmout;
                tv.tv_usec = 0;

                if((n = select(skt_crt + 1, &rcvSet, NULL, NULL, &tv) ) < 0){
                    error_exit("error on select");
                }
                
                if(n == 0) {     // tmout expired, send packet again and remain in this state
                    printf("tmout occured\n");
                    if (send(skt_crt , &pk_s, sizeof(pk_s), 0)== -1){
                    error_exit("sendto()");
                }
                    break;
                }
                
                // socket is readable => ack arrived
                if (recv(skt_crt , &ak_rcv, sizeof(pkt), 0) == -1){
                    error_exit("recvfrom()");
                }               

                if (ak_rcv.seq_no == 1){  // ACK1 has arrived, change the state                        
                    state_symbol = 0;                     
                }
                else{
                    break;
                }

                printf("Received ack seq. no. %d\n\n",ak_rcv.seq_no);                     
            }
            break;
        }
        if(flg_st)
            break;
    }

    
    printf("All packets sent!!\n");

    // close the file and the socket
    fclose(fp);
    close(skt_crt);

    return 0;
}