#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <stdbool.h>



#define RAMS 1024 //Max length of buffer
#define p_no_1 56789
#define p_no_2 9000
#define PDR 10

typedef struct packet Packet;
bool reject;
typedef struct packet{
    int size;
    int val_off;// ye seq no bol ke print karwana hai
    int type;//0 for data packet 1 from ack packet
    char data[RAMS];
    int seq_no;//ye apna scam hai 4 state ke liye fsm me kyunki jarorat to nahi hai

} pkt;

void discardRandom(){
    int n = rand() % 100;
    if( n < PDR){
        reject = true;
    }
    else{
        reject = false;
    }
}
void error_exit(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(){
    Packet rcv_pkt,ack_pkt;
    
    int server_skt1= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_skt1 < 0 )
    {
        printf("Socket2 Creation Error\n");
        exit(0);
    }
    printf("Socket1 Created\n");   // socket done

    int server_skt2= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_skt2 < 0 )
    {
        printf("Socket2 Creation Error\n");
        exit(0);
    }
    printf("Socket2 Created\n");   // socket done
    

    struct sockaddr_in addr_server1,addr_server2,addr_client1,addr_client2;
    memset (&addr_server1, 0, sizeof(addr_server1));
    memset (&addr_server2, 0, sizeof(addr_server2));


    addr_server1.sin_family = AF_INET;
    addr_server1.sin_port = htons(p_no_1);
    // addr_server1.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_server1.sin_addr.s_addr = inet_addr("127.0.0.1");


    addr_server2.sin_family = AF_INET;
    addr_server2.sin_port = htons(p_no_2);
    // addr_server2.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_server2.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf ("Server address assigned\n");
    int server_bind1 = bind(server_skt1, (struct sockaddr*) &addr_server1, sizeof(addr_server1));
    if(server_bind1<0)
    { 
        printf ("Error while binding socket1\n");
        exit (0);
    }
    printf ("Binding successful socket1\n");

    int server_bind2 = bind(server_skt2, (struct sockaddr*) &addr_server2, sizeof(addr_server2));
    if(server_bind2<0)
    { 
        printf ("Error while binding socket2\n");
        exit (0);
    }
    printf ("Binding successful socket2\n");

    int server_listen1= listen(server_skt1,1);
    if(server_listen1<0)
    { 
        printf ("Error while listening 1\n");
        exit (0);
    }
    printf ("Listening successful 1\n");

    int server_listen2= listen(server_skt2,1);
    if(server_listen2<0)
    { 
        printf ("Error while listening 2\n");
        exit (0);
    }
    printf ("Listening successful 2\n");



    int clientLength1 = sizeof(addr_client1);
    int clientSocket1 = accept (server_skt1, (struct sockaddr*) &addr_client1, &clientLength1);
    if (clientLength1 < 0)
    {
        printf ("Error in client socket 1");
        exit(0);
    }
    printf ("Handling Client %s: %d\n", inet_ntoa(addr_client1.sin_addr), ntohs(addr_server1.sin_port));

    int clientLength2 = sizeof(addr_client2);
    int clientSocket2 = accept (server_skt2, (struct sockaddr*) &addr_client2, &clientLength2);
    if (clientLength2 < 0)
    {
        printf ("Error in client socket 2");
        exit(0);
    }
    printf ("Handling Client %s: %d\n", inet_ntoa(addr_client2.sin_addr), ntohs(addr_server2.sin_port));

    FILE* fp = fopen("list.txt", "w");
    if(fp == NULL)
    {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    int state = 0;
    printf("entering while\n");
    while(1)
    {   
        discardRandom();         
        switch(state)
        {
            case 0:;
                printf("in case 0\n");
                memset (&rcv_pkt, 0, sizeof(rcv_pkt));
                int recieve0 = recv(clientSocket1 , &rcv_pkt, sizeof(pkt), 0);

                if(reject==true)
                {
                    printf("Drop Packet  Seq No:%d\n",rcv_pkt.val_off);
                    continue;
                }
                printf("%d",recieve0);
                if(recieve0 < 0)
                {
                    error_exit("revc() error in case 0");
                }   
                
                printf("RCVD PKT:  seq_no: %d,     Size: %d\n",rcv_pkt.val_off, rcv_pkt.size);
                
                if(rcv_pkt.seq_no == 0)
                {   printf("in case 0\n");
                    printf("packet recieved"); 
                    fprintf(fp, "%s,", rcv_pkt.data);
                    fflush(fp);

                    ack_pkt.type = 1;
                    ack_pkt.val_off = rcv_pkt.val_off;
                    ack_pkt.seq_no=0;
                    if (send(clientSocket1,&ack_pkt,sizeof(pkt),0) == -1)
                    {
                        //die();
                        exit(0);
                    }
                    state = 1;
                    printf("SENT ACT: Seq No: %d\n",rcv_pkt.val_off);
                }
                break;

            case 1:;
                printf("in case 1\n");
                memset (&rcv_pkt, 0, sizeof(rcv_pkt));
                int recieve1 = recv ( clientSocket2, &rcv_pkt, sizeof(pkt), 0);
                if(reject==true)
                {
                    // printf("Packet Discarded");
                    continue;
                }
                if(rcv_pkt.seq_no==0)
                {
                    printf("RCVD PKT:  seq_no: %d,     Size: %d\n",rcv_pkt.val_off, rcv_pkt.size);
                
                    if(strcmp(rcv_pkt.data,".")==0)
                    {
                        fclose(fp);
                        exit(0);
                        break;   
                    }
                    fprintf(fp, "%s,", rcv_pkt.data);
                    fflush(fp);

                    ack_pkt.type = 1;
                    ack_pkt.val_off= rcv_pkt.val_off;
                    ack_pkt.seq_no=0;
                    if (send(clientSocket2,&ack_pkt,sizeof(pkt),0) == -1)
                    {
                        //    die();
                        exit(0);
                    }
                    printf("SENT ACT: Seq No: %d\n",rcv_pkt.val_off);
                    state = 2;
                }
                break;
            case 2:;
                printf("in case 2\n");
                memset (&rcv_pkt, 0, sizeof(rcv_pkt));
                int recieve2 = recv ( clientSocket1, &rcv_pkt, sizeof(pkt), 0);
                if(reject==true)
                {
                    // printf("Packet Discarded");
                    continue;
                }
                if(rcv_pkt.seq_no==1)
                {

                    printf("RCVD PKT:  seq_no: %d,     Size: %d\n",rcv_pkt.val_off, rcv_pkt.size);
                
                
                    fprintf(fp, "%s,", rcv_pkt.data);
                    fflush(fp);
                    ack_pkt.type = 1;
                    ack_pkt.seq_no=1;

                    ack_pkt.val_off = rcv_pkt.val_off;
                    if (send(clientSocket1,&ack_pkt,sizeof(pkt),0) == -1)
                    {
                        //die();
                        exit(0);
                    }
                    printf("SENT ACT: Seq No: %d\n",rcv_pkt.val_off);
                    state = 3;
                    
                }
                break;

            case 3:;
                printf("in case 3\n");
                memset (&rcv_pkt, 0, sizeof(pkt));
                int recieve3 = recv ( clientSocket2, &rcv_pkt, sizeof(pkt), 0);
                if(reject==true)
                {
                    // printf("Packet Discarded");
                    continue;
                }
                if(rcv_pkt.seq_no==1)
                {
                   printf("RCVD PKT:  seq_no: %d,     Size: %d\n",rcv_pkt.val_off, rcv_pkt.size);
                
                    if(strcmp(rcv_pkt.data,".")==0)
                    {
                        fclose(fp);
                        exit(0);
                        break;   
                    }
                    printf("seq_no: %d, Name: %s\n",rcv_pkt.seq_no, rcv_pkt.data);
                    fprintf(fp, "%s,", rcv_pkt.data);
                    fflush(fp);

                    ack_pkt.type = 1;
                    ack_pkt.seq_no=1;

                    ack_pkt.val_off= rcv_pkt.val_off;
                    if (send(clientSocket2,&ack_pkt,sizeof(pkt),0) == -1)
                    {
                        //    die();
                        exit(0);
                    }
                    printf("SENT ACT: Seq No: %d\n",rcv_pkt.val_off);
                    state = 0;
                }
                break;
        }
    }
    //close sockets and fclose

  
}