/*
    Client.c 
    UDP Client
*/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "packet.h"

void DieWithError(char *errorMessage);          // Report Error
short alternateNum(short n);                    // Alternate number between 0 and 1: 0 -> 1; 1 -> 0

int main(int argc, char *argv[]) {
    /* Attributes */
    int sock;                           //Socket
    struct sockaddr_in servAddr;        // Server address
    struct sockaddr_in fromAddr;        // Source address
    unsigned short servPort;            // Port
    unsigned int fromSize;              // From size
    char *servIP;                       // Server IP addr
    char *username;                     // Local Username
    short userID;                       // Local UserID
    struct packet pkt;                  // packet to send
    char statusCode = 'E';              // Status Code Error
    char buffer[DATASIZE];              // Buffer for reading input
    struct timeval tv;                  // Time interval
    long micro_t_out;                   // Time in microseconds
    time_t sec_t_out;                   // Time in second
    short seq_num = 0;                  // Sequence Number

    /* Check command line */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server IP> <Port Number>\n", argv[0]);
		exit(1);
    }

    /* Read values */
    servIP = argv[1];
    servPort = atoi(argv[2]);
    printf("Username:\n");
    fgets(buffer, DATASIZE, stdin);
    buffer[strlen(buffer)-1] = '\0'; 
    strcpy(pkt.data, buffer);
    strcpy(username, buffer);

    /* Set Timeout */
    sec_t_out = 100000/1000000;
    micro_t_out = 100;
    tv.tv_sec = sec_t_out;
    tv.tv_usec = micro_t_out; 

    /* set Username packet */
    pkt.seq = htons(seq_num);
    pkt.target_id = htons(0);
    pkt.sender_id = htons(0);
    pkt.opcode = 'U';
    strcpy(pkt.sender, "CLNT");


    /* Create a datagram/UDP socket */
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0) {
        DieWithError("socket() failed");
    }

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port = htons(servPort);

    /* Send username packet to the server */
    if (sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != sizeof(pkt)) {
        DieWithError("send() sent a different number of bytes than expected");
    }
    
    /* Set socket timeout */
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            perror("setsockopt() Error");
    }

    /* Attributes for receiving packets from server */
    struct packet pkt_buff;             // Packet buffer for receiving
    fromSize = sizeof(fromAddr);

    /* Run until user join the char room */
    do {
        // Receive Packet from Server
        while((recvfrom(sock, &pkt_buff, PKTSIZE, 0, (struct sockaddr *) &fromAddr, &fromSize)) < PKTSIZE) {
        }
        statusCode = pkt_buff.opcode;

        if (statusCode == 'E') { // Code E: Error (Username exists / Chat romm full)
            printf("%s\nEnter Username to try again:\n", pkt_buff.data);
            fgets(buffer, DATASIZE, stdin);
            buffer[strlen(buffer)-1] = '\0'; 
            strcpy(pkt.data, buffer);
            strcpy(username, buffer);
            if (sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != sizeof(pkt)) {
                DieWithError("send() sent a different number of bytes than expected");
            }
        } else if (statusCode == 'N') { // Code N: Notification (User join the chatroom)
            userID = ntohs(pkt_buff.target_id); 
            printf("USERNAME: %s\n", username);
            printf("User Created Successfully\n");
            printf("Current users in the chat room:\n\n");
            printf("User ID       Username\n");
            printf("---------------------------------\n");
            char temp[5];
            int temp_id = 1;
            for (int i = 0; i < strlen(pkt_buff.data); i+=4) {
                strncpy(temp, pkt_buff.data+i, 4);
                printf("%d             %s\n",temp_id++, temp);
            }
        }
    } while (statusCode != 'N');
    
    /* Run forever*/
    for (;;) {
        printf("\nEnter <User ID> to send private message\n");
        printf("OR Enter <b> to broadcast\n");
        printf("OR Enter <c> to receive message\n");
        fgets(buffer, DATASIZE, stdin);  
        buffer[strlen(buffer)-1] = '\0';   
        if (strcmp(buffer, "c") == 0) { // Command "c": Check if there are any new messages
            int messageFlag = 0;
            for (int i = 0; i < 5; i++) { // Fetch message 5 times
                if((recvfrom(sock, &pkt_buff, PKTSIZE, 0, (struct sockaddr *) &fromAddr, &fromSize)) != PKTSIZE) { // Timeout: No message
                    messageFlag++;
                } else {    // New Message
                    if (pkt_buff.opcode == 'N') { // Code N: Notification (New User joins chat room, update user list)
                        printf("\nA new user joined the chat room\n");
                        printf("Current users in the chat room:\n\n");
                        printf("User ID       Username\n");
                        printf("---------------------------------\n");
                        char temp[5];
                        int temp_id = 1;
                        for (int i = 0; i < strlen(pkt_buff.data); i+=4) {
                            strncpy(temp, pkt_buff.data+i, 4);
                            printf("%d             %s\n",temp_id++, temp);
                        }

                    } else if (pkt_buff.opcode == 'E') { // Code E: Error (UserID does not exist)
                        printf("\nUserID %d not exist\n", ntohs(pkt.target_id));
                    } else { // Message from users
                        printf("\nFrom %s%s:\n", pkt_buff.sender, pkt_buff.opcode == 'P' ? " (private)" : ""); // Private or broadcast
                        printf("%s\n", pkt_buff.data);
                    }   
                } 
                if (messageFlag >= 5) { // No new Message
                    printf("No New Messages\n");
                }
            }
 
        } else {
            seq_num = alternateNum(seq_num);
            if (strcmp(buffer, "b") == 0) { // Command "b": Broadcast message: set target id to 9999
                pkt.target_id = htons(9999);
            } else { // Digit input: set target to the number
                pkt.target_id = htons(atoi(buffer));
            }
            // Enter message to send
            printf("\nEnter Message:\n");
            fgets(buffer, DATASIZE, stdin);
            buffer[strlen(buffer)-1] = '\0';
            strcpy(pkt.data, buffer);
            strcpy(pkt.sender, username);
            pkt.seq = htons(seq_num); 
            pkt.sender_id = htons(userID);
            pkt.opcode = 'M';

            /* Send message packet to the server */
            if (sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != sizeof(pkt)) {
                DieWithError("send()Message sent a different number of bytes than expected");
            }
        }
    }

    
        
}