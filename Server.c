/*
    Server.c
    Server
*/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "packet.h"

typedef struct {
    char username[81];                  // Username
    short userID;                       // UserID
    struct sockaddr_in clntAddr;        // Client address
    unsigned int cliAddrLen;            // Client address Length
    short seq;                    // Sequence Number to the specific host
} User;

void DieWithError(char *errorMessage);
void HandleClient(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, struct packet in_packet, User * users, int* currentUsers, int max_capacity);
short alternateNum(short n);
void sendMessage(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, char * message, char opcode, short target_id, char * sender, short seq);
char * getUserList(User * users, int currentUsers);

int main(int argc, char *argv[]) {

    /* Attributes */
    int sock;                           // Socket
    struct sockaddr_in servAddr;        // Server address
    struct sockaddr_in clntAddr;        // Client address
    unsigned int cliAddrLen;            // Length of incoming message
    unsigned short servPort;            // Server port
    int broadcastEnable = 1;            // broadcast Enable
    struct packet pkt_buff;             // Packet buffer
    int max_capacity;                   // Max Chat Room Capacity
    int currentUsers;
    

    /* Check command line input */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server Port> <Max Chat Room Capacity>\n", argv[0]) ;
		exit(1);
    }
    printf("Initiating Server...\n");
    
    /* Assign values */
    servPort = atoi(argv[1]);
    max_capacity = atoi(argv[2]);
    User users[max_capacity];
    currentUsers = 0;

    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("socket() failed");
    }

    /* Set Socket, Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    /* Bind the local address */
    if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        DieWithError(" bind() failed");
    }

    /* Run Forever */
    for (;;) {
        // Set the size of the in-out parameter
        cliAddrLen = sizeof(clntAddr);

        // Receive packet request
        if ((recvfrom(sock, &pkt_buff, PKTSIZE, 0, (struct sockaddr *) &clntAddr, &cliAddrLen)) < 0) {
            DieWithError("filename recvfrom() failed");
        }
        
        // Handle the request
        HandleClient(sock, clntAddr, cliAddrLen, pkt_buff, users, &currentUsers, max_capacity);
    }
        
}


void HandleClient(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, struct packet in_packet, User * users, int* currentUsers, int max_capacity) {
    printf("Handling....\n");
    /* Temp variable */
    struct sockaddr_in tempclntAddr;
    unsigned int tempcliAddrLen;

    /* Packet attributes */
    char opcode = in_packet.opcode;
    short target = ntohs(in_packet.target_id);
    short sender_id = ntohs(in_packet.sender_id);

    /* Respond based on request opcode */
    switch(opcode) {
        case 'U': // Code U: Create user in server
            if (*currentUsers >= max_capacity) { // Room Full 
                printf("Chat room full\n");
                sendMessage(servSocket, clntAdd, clnAddrLen, "Chat room full", 'E', 0, "SRVR", 0);
                return;
            }

            for (int i = 0; i < *currentUsers; i++) { // Username existed
                if (strcmp(users[i].username, in_packet.data) == 0) {
                    printf("User name existed\n");
                    sendMessage(servSocket, clntAdd, clnAddrLen, "Username Existed", 'E', 0, "SRVR", 0);
                    return;
                }
            }

            // Creating user...
            strcpy(users[*currentUsers].username, in_packet.data);
            users[*currentUsers].clntAddr = clntAdd;
            users[*currentUsers].cliAddrLen = clnAddrLen;
            users[*currentUsers].seq = 0;
            *currentUsers += 1;
            users[*currentUsers].userID = *currentUsers;
            
            char * userlist = getUserList(users, *currentUsers);
            sendMessage(servSocket, clntAdd, clnAddrLen, userlist, 'N', users[*currentUsers].userID, "SRVR", users[*currentUsers].seq);
            users[*currentUsers].seq = alternateNum(users[*currentUsers].seq);
            for (int i = 0; i < *currentUsers-1; i++) {
                sendMessage(servSocket, users[i].clntAddr, users[i].cliAddrLen, userlist, 'N', users[i].userID, "SRVR", users[i].seq);
                users[i].seq = alternateNum(users[i].seq);
            }
            free(userlist);
            break;

        case 'M': // Code M: Message (Send Message to users)
            if (target == 9999) { // Broadcast
                for (int i = 0; i < *currentUsers; i++) {
                    if ((i+1) != sender_id) {
                        sendMessage(servSocket, users[i].clntAddr, users[i].cliAddrLen, in_packet.data, 'B', users[i].userID, in_packet.sender, users[i].seq);
                        users[i].seq = alternateNum(users[i].seq);
                    }
                }
            } else { // Unicast
                
                if (target < 1 || target > *currentUsers) { // Invalid UserID
                    sendMessage(servSocket, clntAdd, clnAddrLen, "UserID Not Exist", 'E', target, in_packet.sender, users[sender_id-1].seq);
                    users[sender_id-1].seq = alternateNum(users[sender_id-1].seq);
                    return;
                }
                // Send message to target user
                tempclntAddr = users[target-1].clntAddr;
                tempcliAddrLen = users[target-1].cliAddrLen;
                sendMessage(servSocket, tempclntAddr, tempcliAddrLen, in_packet.data, 'P', target, in_packet.sender, users[target-1].seq);
                users[target-1].seq = alternateNum(users[target-1].seq);
            }
            break;
        default: //Error
            printf("Failed: Unidentified Opcode\n");
    }
}

// Send Message
void sendMessage(int servSocket, struct sockaddr_in clntAdd, unsigned int clnAddrLen, char * message, char opcode, short target_id, char * sender, short seq) {
    struct packet temp_pkt;
    strcpy(temp_pkt.data, message);
    temp_pkt.opcode = opcode;
    temp_pkt.target_id = htons(target_id);
    temp_pkt.sender_id = htons(0);
    temp_pkt.seq = htons(seq);
    strcpy(temp_pkt.sender, sender);
    if (sendto(servSocket, &temp_pkt, sizeof(temp_pkt), 0, (struct sockaddr *) &clntAdd, clnAddrLen) != sizeof(temp_pkt)) {
        DieWithError("sendto() sent a different number of bytes than expected");
    }
}

// Get Current Users active in the server
char * getUserList(User * users, int currentUsers) {
    char * userList = malloc(81);
    strcpy(userList, "");
    for (int i = 0; i < currentUsers; i++) {
        strcat(userList, users[i].username);
    }
    return userList;
}
