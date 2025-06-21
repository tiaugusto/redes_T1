CFLAGS=-Wall -g
LIBS=-lncurses -ltinfo -lpthread

all: server client

server: server.c protocol_net.o protocol.o socket.o ui_server.o utils.o
	$(CC) $(CFLAGS) -o server \
		server.c protocol_net.o protocol.o socket.o ui_server.o utils.o $(LIBS)

client: client.c protocol_net.o protocol.o socket.o ui_client.o
	$(CC) $(CFLAGS) -o client \
		client.c protocol_net.o protocol.o socket.o ui_client.o $(LIBS)

protocol.o: protocol.c protocol.h
	$(CC) $(CFLAGS) -c protocol.c

protocol_net.o: protocol_net.c protocol_net.h protocol.h socket.h
	$(CC) $(CFLAGS) -c protocol_net.c

socket.o: socket.c socket.h
	$(CC) $(CFLAGS) -c socket.c

ui_server.o: ui_server.c ui_server.h
	$(CC) $(CFLAGS) -c ui_server.c

ui_client.o: ui_client.c ui_client.h
	$(CC) $(CFLAGS) -c ui_client.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o server client
