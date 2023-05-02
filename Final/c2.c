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
#define p_s 9000
#define TIMEOUT 2 // retransmission timeout duration

bool reject;

typedef struct packet
{
    int size;
    int offset; // storing the offset of file pointer
    int type;   // 0 for data packet 1 from ack packet
    char data[SIZE_B];
    int seq_no;
} pkt;

void error_exit(char *abc)
{
    perror(abc);
    exit(EXIT_FAILURE);
}

int main()
{
    // create a TCP socket
    int crt_skt = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (crt_skt == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set up the server address and port
    struct sockaddr_in add_s;
    add_s.sin_family = AF_INET;
    add_s.sin_addr.s_addr = inet_addr("127.0.0.1");
    add_s.sin_port = htons(p_s);

    
    if (connect(crt_skt, (struct sockaddr *)&add_s, sizeof(add_s)) != 0)
    {
        perror("Connection with the server failed");
        exit(EXIT_FAILURE);
    }

    
    FILE *fp = fopen("id.txt", "r");
    if (fp == NULL)
    {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    
    struct packet pkt_snd, ak_rec;
    int s_flag = 0;

    int switch_symbol = 0;
    
    while (!feof(fp))
    {
        
        switch (switch_symbol)
        {
        case 0:
        {
            char file_name_val[256];
            if (fscanf(fp, "%[^,.],", file_name_val) != 1)
            {
                s_flag = 1;
                break; // end of file
            }
            printf("%s\n", file_name_val);
            memset(&pkt_snd, 0, sizeof(pkt_snd));
            
            pkt_snd.type = 0;   
            pkt_snd.seq_no = 0; 
            pkt_snd.size = strlen(file_name_val);
            strcpy(pkt_snd.data, file_name_val);

            
            int df = send(crt_skt, &pkt_snd, sizeof(pkt), 0);
            if (df < 0)
            {
                error_exit("sendto()");
            }
            printf("pkt 1 -> name: %s, seq_no: %d\n", pkt_snd.data, pkt_snd.seq_no);
            switch_symbol = 1;
        }
        break;

        case 1:
        { // waiting for ACK 0
            // printf("In case 1\n");

            fd_set Srcv;
            int n;

            struct timeval tv;

            FD_ZERO(&Srcv);
            FD_SET(crt_skt, &Srcv);

            tv.tv_sec = TIMEOUT;
            tv.tv_usec = 0;

            if ((n = select(crt_skt + 1, &Srcv, NULL, NULL, &tv)) < 0)
            {
                error_exit("error on select");
            }

            if (n == 0)
            { 
                printf("Timeout occured\n");
                if (send(crt_skt, &pkt_snd, sizeof(pkt_snd), 0) == -1)
                {
                    error_exit("sendto()");
                }
                break;
            }

            
            if (recv(crt_skt, &ak_rec, sizeof(pkt), 0) == -1)
            {
                error_exit("recv()");
            }
            printf("Ack1 received\n");
            if (ak_rec.seq_no == 0)
            { // if ACK0 arrived, change the state
                switch_symbol = 2;
            }
            else
            {
                break;
            }

            printf("Received ack seq. no. %d\n\n", ak_rec.seq_no);
        }
        break;

        case 2:
        {
            char file_name_val[256];
            if (fscanf(fp, "%[^,.],", file_name_val) != 1)
            {
                s_flag = 1;
                break; // end of file
            }
            printf("%s\n", file_name_val);
            memset(&pkt_snd, 0, sizeof(pkt_snd));

            
            pkt_snd.type = 0; // data packet

            pkt_snd.size = strlen(file_name_val);
            memcpy(pkt_snd.data, file_name_val, pkt_snd.size);
            

            pkt_snd.seq_no = 1;
            if (send(crt_skt, &pkt_snd, sizeof(pkt), 0) == -1)
            {
                error_exit("sendto()");
            }
            switch_symbol = 3;
        }
        break;

        case 3: // waiting for ACK 1
        {
            fd_set Srcv;
            int n;

            struct timeval tv;

            FD_ZERO(&Srcv);
            FD_SET(crt_skt, &Srcv);

            tv.tv_sec = TIMEOUT;
            tv.tv_usec = 0;

            if ((n = select(crt_skt + 1, &Srcv, NULL, NULL, &tv)) < 0)
            {
                error_exit("error on select");
            }

            if (n == 0)
            { // timeout expired, send packet again and remain in this state
                printf("Timeout occured\n");
                if (send(crt_skt, &pkt_snd, sizeof(pkt_snd), 0) == -1)
                {
                    error_exit("sendto()");
                }
                break;
            }

            // socket is readable => ack arrived
            if (recv(crt_skt, &ak_rec, sizeof(pkt), 0) == -1)
            {
                error_exit("recvfrom()");
            }

            if (ak_rec.seq_no == 1)
            { // ACK1 has arrived, change the state
                switch_symbol = 0;
            }
            else
            {
                break;
            }

            printf("Received ack seq. no. %d\n\n", ak_rec.seq_no);
        }
        break;
        }
        if (s_flag)
            break;
    }

    
    printf("All packets sent!!\n");

    
    fclose(fp);
    close(crt_skt);

    return 0;
}