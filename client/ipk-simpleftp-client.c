#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <netdb.h>
#include "clientConfig.h"
#include <sys/socket.h>
#include <signal.h>

#define NONE -1
#define RECIVE 1
#define SEND 2
#define SA struct sockaddr

int argumentHandler(int argc, char *argv[], ClientConfig *clientConfig);
void printUsage();
void printHelp();
void printLetMeIn();
int analyze(char *line, Stash *stash, int sockfd, ClientConfig *clientConfig);

void catchSignal(int signal) {
  if (signal == SIGINT) {
    close(115);
    exit(0);
  }
}

int main(int argc, char **argv) {
  int sockfd,  n;
  signal(SIGINT, catchSignal);
  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *rp;
  int s;
  ClientConfig *clientConfig = malloc(sizeof(ClientConfig));
  initConfig(clientConfig);

  int rc = argumentHandler(argc, argv, clientConfig);

  Stash *stash = malloc(sizeof(Stash));
  initStash(stash);

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

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;


  s = getaddrinfo(clientConfig->ip, clientConfig->port, &hints, &result);
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

  char *line = malloc(sizeof(char) * 1000);
  int responseCode;


  n = recv(sockfd, line, 1000, 0);
  if (n < 0) {
    fprintf(stderr, "ERROR reading from socket");
    return 1;
  }

  printf("%s\n", line);

  while(true) {
    memset(line, 0, 1000);
    fgets(line, 1000, stdin);
    line[strlen(line)-1] = '\0';
    responseCode = analyze(line, stash, sockfd, clientConfig);
    if(responseCode == 1) {
      close(sockfd);
      break;
    } else if(responseCode == 2) {
      n = send(sockfd,line, strlen(line)+1, 0);
      if(n < 0) {
        fprintf(stderr, "ERROR writing to socket");
      }

      memset(line, 0, 1000);
      n = recv(sockfd, line, 1000, 0);

      if (n < 0) {
        fprintf(stderr, "ERROR reading from socket");
        return 1;
      }

      printf("%s\n", line);

      if(line[0] == '-') {
        stash->operation = NONE;
        strcpy(stash->stash, "");
      } else if(stash->operation == RECIVE) {
        stash->length = (int) strtol(line, (char **) NULL, 10);
      }

    }
  }

  return 0;
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
        strcpy(clientConfig->port, optarg);
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

int analyze(char *line, Stash *stash, int sockfd, ClientConfig *clientConfig) {
  char help[100];

  if(strcmp(line, "DONE") == 0) {
    return 1;
  } else if(strcmp(line, "HELP") == 0) {
    printHelp();
    return 0;
  } else if(strcmp(line, "LET-ME-IN") == 0) {
    printLetMeIn();
    return 0;
  } else {
    char buff[100];
    strcpy(buff, line);
    char *split = strtok(buff, " ");
    if((strcmp(split, "HOME") == 0) || (strcmp(split, "LIST") == 0) || (strcmp(split, "CDIR") == 0) ||
      (strcmp(split, "KILL") == 0) || (strcmp(split, "NAME") == 0) || (strcmp(split, "TOBE") == 0) ||
      (strcmp(split, "TYPE") == 0) ||
      (strcmp(split, "STOR") == 0) || (strcmp(split, "SIZE") == 0) ||
      (strcmp(split, "USER") == 0) || (strcmp(split, "ACCT") == 0) || (strcmp(split, "PASS") == 0)) {
      return 2;
    } else if(strcmp(split, "RETR") == 0) {
      split = strtok(NULL, " ");

      if(split == NULL) {
        printf("Use command HELP for help.\n");
        printf("Use command LET-ME-IN for help with log in.\n");
        return 0;
      }
      stash->operation = SEND;
      strcpy(stash->stash, split);
      return 2;
    } else if(strcmp(split, "SEND") == 0) {
      printf("send\n");
      int n = send(sockfd,"SEND", 5, 0);
      if(n < 0) {
        fprintf(stderr, "ERROR writing to socket");
      }
      strcpy(help, clientConfig->workingDirectory);

      if (clientConfig->workingDirectory[strlen(clientConfig->workingDirectory)-1] != '/') {
        strcat(help, "/");
      }

      strcat(help, stash->stash);

        FILE *fp = fopen(help, "wb");
        int tot = 0;
        int b = 0;

      if(fp != NULL){
        while( (b = recv(sockfd, buff, 1000,0)) > 0 ) {
          tot+=b;
          fwrite(buff, 1, b, fp);
        }
        fclose(fp);
      }
      printf("sendEND\n");
      return 0;
    } else if(strcmp(split, "STOP") == 0) {
      stash->operation = NONE;
      strcpy(stash->stash, "");
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
  printf("\t<cmd> : =  USER ! ACCT ! PASS ! TYPE ! LIST ! CDIR ! KILL ! NAME ! DONE ! RETR ! STOR ! HOME\n");
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