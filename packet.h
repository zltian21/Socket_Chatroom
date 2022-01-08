#define PKTSIZE 96
#define DATASIZE 81

/* Data Packet */
struct packet {
    char data[81];          // Message
    char sender[5];         // Sender Username (has to be 4 characters)
    short seq;              // Packet sequence number
    short target_id;        // Target userID
    short sender_id;        // Sender userID
    char opcode;            // Operation Code
    char padding[3];        // Padding
};


/*  +++++++Variable Usage++++++++

    short target_id/sender_id 
        0: Default
        >0: User ID
        9999: Boardcast

    char sender[5] 
        Sender user name 4 char
        Client: "CLNT"
        Server: "SRVR"

    char opcode
        'U': Create User                   Client -> Server
        'E': Error Message            Server -> Client
        'N': Notification                  Server -> Client
        'P': Private                       Server -> Client
        'B': Broadcast                     Server -> Client
*/
