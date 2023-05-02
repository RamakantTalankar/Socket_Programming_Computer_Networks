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

#define BUFLEN 1024
#define SERVER_PORT 9000
#define TIMEOUT 2 // retransmission timeout duration

bool toDiscard;

// struct packet {
//     uint8_t type; // 0 for data, 1 for ACK
//     uint32_t seq_no; // sequence number of the packet
//     uint16_t payload_size; // size of the payload in bytes
//     char payload[MAX_PAYLOAD_SIZE]; // payload of the packet
// };

typedef struct packet{
    int size;
    int offset;// ye seq no bol ke print karwana hai
    int type;//0 for data packet 1 from ack packet
    char data[BUFLEN];
    int seq_no;//ye apna scam hai 4 state ke liye fsm me kyunki jarorat to nahi hai
} pkt;

void error_exit(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}


int main() {
    // create a TCP socket
    int sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set up the server address and port
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(SERVER_PORT);

    // connect to the server
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connection with the server failed");
        exit(EXIT_FAILURE);
    }

    // // read the file name from the user
    // char file_name[256];
    // printf("Enter the name of the file to be sent: ");
    // scanf("%s", file_name);

    // open the file for reading
    FILE* fp = fopen("id.txt", "r");
    if (fp == NULL) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    // // send the file name to the server
    // struct packet pkt;
    // pkt.type = 0; // data packet
    // pkt.seq_no = 0; // sequence number
    // pkt.payload_size = strlen("name.txt");
    // memcpy(pkt.payload, "name.txt", pkt.payload_size);
    // send(sockfd, &pkt, sizeof(pkt), 0);
    // printf("SENT PKT: Seq. No. = %d, Size = %d Bytes\n", pkt.seq_no, pkt.payload_size);

    // // wait for the acknowledgement from the server
    // struct packet ack_pkt;
    // recv(sockfd, &ack_pkt, sizeof(ack_pkt), 0);
    // printf("RCVD ACK: Seq. No. = %d\n", ack_pkt.seq_no);

    // start sending the file contents to the server
    struct packet send_pkt, rcv_ack;
    int offset=0;
    int state = 0;
    // int seq_no = 0;
    while (!feof(fp)) {
        // read a student name from the file
        switch(state)
        {
            case 0: 
            {
                char student_name[256];
                if (fscanf(fp, "%[^,.],", student_name) != 1) {
                    break; // end of file
                }
                printf("%s\n", student_name);
                memset(&send_pkt,0,sizeof(send_pkt));
                // create a data packet for the student name
                send_pkt.offset=offset;
                send_pkt.type = 0; // data packet
                send_pkt.seq_no = 0; // sequence number
                send_pkt.size = strlen(student_name);
                strcpy(send_pkt.data, student_name);

                //printf("In case 0\n");
                //
                // printf("Enter message 0: ");//wait for sending packet with seq. no. 0  
                int socket_fd = send(sockfd , &send_pkt, sizeof(pkt), 0);
                if ( socket_fd < 0)
                {
                    error_exit("sendto()");
                }
                printf("SENT PKT :Seq.No %d,Size %d\n",send_pkt.offset,send_pkt.size);
                offset+=send_pkt.size;
                state = 1;
            }
            break; 

            case 1:
            {//waiting for ACK 0
                printf("In case 1\n");


                fd_set rcvSet;
                int n;
                
                struct timeval tv;

                FD_ZERO(&rcvSet);
                FD_SET(sockfd, &rcvSet);

                tv.tv_sec = TIMEOUT;
                tv.tv_usec = 0;

                if((n = select(sockfd + 1, &rcvSet, NULL, NULL, &tv) ) < 0){
                    error_exit("error on select");
                }
                
                if(n == 0) {     // timeout expired, send packet again and remain in this state
                    printf("Timeout occured\n");
                    if (send(sockfd , &send_pkt, sizeof(send_pkt), 0)== -1){
                    error_exit("sendto()");
                    }
                     printf("RE-TRANSMIT PKT: Seq. No.  %d, Size = %d\n",send_pkt.offset,send_pkt.size);
                    break;
                }
                
                // socket is readable => ack arrived
                printf("yet to recv ack1\n");
                if (recv(sockfd , &rcv_ack, sizeof(pkt), 0) == -1){
                    error_exit("recv()");
                }               
                 printf("RCVD ACK Seq No:%d\n",rcv_ack.offset);
                if(rcv_ack.seq_no == 0){  // if ACK0 arrived, change the state                        
                    state = 2;                    
                }
                else{
                    break;
                }

                
            }
            break;

            case 2:
            {
                char student_name[256];
                if (fscanf(fp, "%[^,.],", student_name) != 1) {
                    break; // end of file
                }
                printf("%s\n", student_name);
                memset(&send_pkt,0,sizeof(send_pkt));
                // create a data packet for the student name
                send_pkt.offset=offset;
                send_pkt.type = 0; // data packet
                // send_pkt.seq_no = seq_no; // sequence number
                send_pkt.size = strlen(student_name);
                memcpy(send_pkt.data, student_name, send_pkt.size);
                printf("In case 2\n");

                // printf("Enter message 1: ");  
                //wait for sending packet with seq. no. 1
                // fgets(send_pkt.data, BUFLEN, stdin);

                send_pkt.seq_no = 1;     
                if (send(sockfd , &send_pkt, sizeof(pkt), 0)== -1){
                    error_exit("sendto()");
                }
                offset+=send_pkt.size;
                printf("SENT PKT :Seq.No %d,Size %d\n",send_pkt.offset,send_pkt.size);
                state = 3; 
            }
            break;                    

            case 3: //waiting for ACK 1
            {
                printf("In case 3\n");


                fd_set rcvSet;
                int n;
                
                struct timeval tv;

                FD_ZERO(&rcvSet);
                FD_SET(sockfd, &rcvSet);

                tv.tv_sec = TIMEOUT;
                tv.tv_usec = 0;

                if((n = select(sockfd + 1, &rcvSet, NULL, NULL, &tv) ) < 0){
                    error_exit("error on select");
                }
                
                if(n == 0) {     // timeout expired, send packet again and remain in this state
                    printf("Timeout occured\n");
                    if (send(sockfd , &send_pkt, sizeof(send_pkt), 0)== -1){
                    error_exit("sendto()");
                }
                printf("RE-TRANSMIT PKT: Seq. No.  %d, Size = %d\n",send_pkt.offset,send_pkt.size);
                    break;
                }
                
                // socket is readable => ack arrived
                if (recv(sockfd , &rcv_ack, sizeof(pkt), 0) == -1){
                    error_exit("recvfrom()");
                }               

                if (rcv_ack.seq_no == 1){  // ACK1 has arrived, change the state                        
                    state = 0;                     
                }
                else{
                    break;
                }

                printf("Received ack seq. no. %d\n\n",rcv_ack.seq_no);                     
            }
             printf("RCVD ACK Seq No:%d\n",rcv_ack.offset);                     
            break;
        }
        
    }

    // // send an end-of-file packet to the server
    // struct packet eof_pkt = {0};
    // eof_pkt.type = 0; // data packet
    // eof_pkt.seq_no = seq_no; // sequence number
    // eof_pkt.payload_size = 0;
    // send(sockfd, &eof_pkt, sizeof(eof_pkt), 0);
    // printf("SENT PKT: Seq. No. = %d, Size = %d Bytes\n", eof_pkt.seq_no, eof_pkt.payload_size);

    // // wait for the acknowledgement from the server
    // recv(sockfd, &ack_pkt, sizeof(ack_pkt), 0);
    // printf("RCVD ACK: Seq. No. = %d\n", ack_pkt.seq_no);

    printf("All packets sent!!\n");

    // close the file and the socket
    fclose(fp);
    close(sockfd);

    return 0;
}