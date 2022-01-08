
List of relevant files:
    Makefile
    packet.h
    DieWithError.c
    Simulation.c
    Client.c
    Server.c

Compilation instruction:
    make

Running instruction:
    Running Server:
        ./Server <Server Port> <Chat Room Capacity>
        Example: ./Server 9999 3

    Running Client:
        ./Client <Server IP> <Port Number>
        Example: ./Client 127.0.0.1 9999

!!!!! IMPORTANT NOTIFICATION:
1. Username has to be 4 characters

2. Enter userID to send message (NOT username)

2. Enter c to check new messages
Since this is a single thread program,
the client needs manually check all messages by entering the "c" command in the terminal.
And this includes private/broadcast message, userID do not exist message, and user list update message. 
    
    
