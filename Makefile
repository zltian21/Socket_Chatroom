CC=gcc
CFLAGS=Wall

all: Server Client

Server: Server.o DieWithError.o Simulation.o

Client: Client.o DieWithError.o Simulation.o

DieWithError.o: DieWithError.c
	$(CC) -c DieWithError.c

Simulation.o: Simulation.c
	$(CC) -c Simulation.c

Server.o: Server.c packet.h
	$(CC) -c Server.c

Client.o: Client.c packet.h
	$(CC) -c Client.c

clean:
	rm -f Server Client *.o