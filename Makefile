CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g -Wno-unused-parameter

CLIENT=ipk-simpleftp-client.c
CLIENT_LIBS=clientConfig.c
SERVER=ipk-simpleftp-server.c
SERVER_LIBS=serverConfig.c

.PHONY: all
all:
	$(CC) $(CLIENT) $(CLIENT_LIBS) -o ipk-simpleftp-client $(CFLAGS) -g
	$(CC) $(SERVER) $(SERVER_LIBS) -o ipk-simpleftp-server $(CFLAGS) -g


.PHONY: run
run: all
	./hinfosvc