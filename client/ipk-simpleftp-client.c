#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "clientConfig.h"
#include <sys/socket.h>
#include <netdb.h>

#define MAX 80
#define PORT 115
#define SA struct sockaddr

int argumentHandler(int argc, char *argv[], ClientConfig *clientConfig);
void printUsage();
void printHelp();
void printLetMeIn();
int analyze(char *line);

int main(int argc, char **argv) {
  int sockfd,  n;
  struct addrinfo hints;
  char buffer[256];
  struct addrinfo *result;
  struct addrinfo *rp;
  char *portA = "115";
  int s;
  //char *serverIP = "fe80::f55b:6004:5817:95e5";
  char *serverIP = "2a02:8308:a085:7900:db6d:77f6:3ff0:e92a";

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;


  s = getaddrinfo(serverIP, portA, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return 1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1) {
      continue;
    }

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }

    close(sockfd);
  }

  if (rp == NULL) {
    fprintf(stderr, "Could not find the right address to connect\n");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(result);

  char *line = malloc(sizeof(char) * 110);
  int responseCode;

  while(true) {
    memset(line, 0, 100);
    fgets(line, 100, stdin);
    responseCode = analyze(line);
    if(responseCode == 1) {
      close(sockfd);
      break;
    } else if(responseCode == 2) {
      n = send(sockfd,line, strlen(line)+1, 0);
      if(n < 0) {
        fprintf(stderr, "ERROR writing to socket");
      }

      memset(line, 0, 100);
      n = recv(sockfd, line, 100, 0);

      if (n < 0) {
        fprintf(stderr, "ERROR reading from socket");
      }
      printf("%s\n", line);
    }
  }

  return 0;
    /*int flag_off = 0;
    int sockfd, connfd;
    struct addrinfo hints;
    struct sockaddr_in6 servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");


    bzero(&servaddr, sizeof(servaddr));
    // assign IP, PORT
    servaddr.sin6_family = AF_UNSPEC;
    //servaddr.sin6_addr = inet_addr("192.168.56.102");
    inet_pton(AF_INET6, "fe80::f55b:6004:5817:95e5", &servaddr.sin6_addr);
    servaddr.sin6_port = htons(PORT);
    servaddr.sin6_flowinfo = 0;
    servaddr.sin6_scope_id = 0;

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
    return 0;*/
  /*bool loggedIn = false;
  bool tobe = false;
  bool sendB = false;
  ClientConfig *clientConfig = malloc(sizeof(ClientConfig));
  initConfig(clientConfig);

    int flag = 1;
    int flag_off = 0;

  int rc = argumentHandler(argc, argv, clientConfig);

  if(rc == 1) {
    printf("Bad arguments.\n");
    printf("Use parameter -H to get help.\n");
    destroyConfig(&clientConfig);
    free(clientConfig);
    return 1;
  } else if(rc == 2) {
    destroyConfig(&clientConfig);
    free(clientConfig);
    return 0;
  }

  int socket_desc;
  struct sockaddr_in server_addr;
  char server_message[2000], client_message[2000];

    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("192.168.56.102");

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    // Get input from the user:
    printf("Enter message: ");
    fgets(client_message, 1000, stdin);

    // Send the message to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }

    // Receive the server's response:
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(socket_desc);

    return 0;

  char *line = malloc(sizeof(char) * 110);
  int responseCode;
  while(true) {
    fgets(line, 100, stdin);
    responseCode = analyze(line, &loggedIn, &tobe, &sendB);
    if(responseCode == 1) {
      break;
    } else if(responseCode == -1) {
      printf("Not a valid command.\n");
      printf("Use HELP for help.\n");
      printf("Use LET-ME-IN for help with logging in.\n");
    }
  }

  free(line);
  destroyConfig(&clientConfig);
  free(clientConfig);

  return 0;*/
}

int argumentHandler(int argc, char *argv[], ClientConfig *clientConfig) {
  int option;
  char *p;

  while ((option = getopt(argc, argv, "h:p:f:H")) != -1) {
    switch (option) {
      case 'h':
        strcpy(clientConfig->ip, optarg);
        break;
      case 'p':
        clientConfig->port = (int) strtol(optarg, &p, 10);
        break;
      case 'f':
        strcpy(clientConfig->workingDirectory, optarg);
        break;
      case 'H':
        printUsage();
        return 2;
        break;
      default:
        return 1;
        break;
    }
  }

  if(strlen(clientConfig->ip) == 0 || strlen(clientConfig->workingDirectory) == 0) {
    return 1;
  }

  return 0;
}

int analyze(char *line) {
  line[strlen(line)-1] = '\0';
  if(strcmp(line, "DONE") == 0) {
    return 1;
  } else if(strcmp(line, "HELP") == 0) {
    printHelp();
    return 0;
  } else if(strcmp(line, "LET-ME-IN") == 0) {
    printLetMeIn();
    return 0;
  } else {

    if((strcmp(line, "HOME") == 0) || (strcmp(line, "LIST") == 0) || (strcmp(line, "CDIR") == 0) ||
      (strcmp(line, "KILL") == 0) || (strcmp(line, "NAME") == 0) || (strcmp(line, "TOBE") == 0) ||
      (strcmp(line, "TYPE") == 0) || (strcmp(line, "RETR") == 0) || (strcmp(line, "SEND") == 0) ||
      (strcmp(line, "STOP") == 0) || (strcmp(line, "STOR") == 0) || (strcmp(line, "SIZE") == 0)) {

      return 2;

    } else {
      printf("Use command HELP for help.\n");
      printf("Use command LET-ME-IN for help with log in.\n");
      return 0;
    }
  }
}

// Printing zone
//----------------------------------------------------------------------------------------------------

void printUsage() {
  printf("Simple File Transfer Protocol - client\n");
  printf("Usage:\n");
  printf("./ipk-simpleftp-client {-h server IP} {-p port} [-f path to working directory]\n");
  printf("-h server IP - IPv4 or IPv6 address supported - required parameter\n");
  printf("-p port - if not provided, default value is 115\n");
  printf("-f path to working directory on server - required parameter\n\n");
  printf("Description:\n");
  printf("This server works just with ipk-simpleftp-server\n\n");
  printf("Author: Alexander Okrucký [xokruc00]\n");
}

void printHelp() {
  printf("------------------------------------------------------------------------------------------\n");
  printf("Usage:\n");
  printf("\t<command> : = <cmd> [<SPACE> <args>] <NULL>\n");
  printf("\t<cmd> : =  USER ! ACCT ! PASS ! TYPE ! LIST ! CDIR ! KILL ! NAME ! DONE ! RETR ! STOR\n");
  printf("\t<response> : = <response-code> [<message>] <NULL>\n");
  printf("\t<response-code> : =  + | - |   | !\n");
  printf("\t<message> can contain <CRLF>\n");
  printf("\nMore information about commands is in documentation.\n");
  printf("\nFor help how to log in type LET-ME-IN.\n");
  printf("------------------------------------------------------------------------------------------\n");
}

void printLetMeIn() {
  printf("------------------------------------------------------------------------------------------\n");
  printf("How to log in:\n");
  printf("You can first type password or username, it is up to you.\n\n");
  printf("USER user-name\n");
  printf("\t!<user-name> logged in -> no password needed\n");
  printf("\t+user-name valid -> need to send password\n");
  printf("\t-Invalid user-name -> user with given user-name does not exist\n\n");
  printf("ACCT user-name\n");
  printf("\t!<user-name> logged in -> no password needed\n");
  printf("\t+user-name valid -> send password, next send password\n");
  printf("\t-Invalid user-name -> user with given user-name does not exist\n\n");
  printf("PASS password\n");
  printf("\t! Logged in -> password is ok and u can start transfer\n");
  printf("\t+Send account -> password is ok but you have not specified account\n");
  printf("\t-Wrong password -> bad password\n\n");
  printf("More information in documentation.\n");
  printf("For help use HELP.\n");
  printf("------------------------------------------------------------------------------------------\n");
}