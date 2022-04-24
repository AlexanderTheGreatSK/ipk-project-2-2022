CC=gcc

CLIENT=./client/ipk-simpleftp-client.c
CLIENT_LIBS=./client/clientConfig.c
SERVER=./server/ipk-simpleftp-server.c
SERVER_LIBS=./server/serverConfig.c ./server/auth.c

.PHONY: all
all:
	$(CC) $(CLIENT) $(CLIENT_LIBS) -o ipk-simpleftp-client
	$(CC) $(SERVER) $(SERVER_LIBS) -o ipk-simpleftp-server
