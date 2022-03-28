#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#include "clientConfig.h"

int argumentHandler(int argc, char *argv[], ClientConfig *clientConfig);
void printUsage();
int analyze(char *line, bool *loggedIn, bool *tobe, bool *send);

int main(int argc, char **argv) {
  printf("hello from client\n");

  bool loggedIn = false;
  bool tobe = false;
  bool send = false;
  ClientConfig *clientConfig = malloc(sizeof(ClientConfig));
  initConfig(clientConfig);

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

  char *line = malloc(sizeof(char) * 110);
  int responseCode;
  while(true) {
    fgets(line, 100, stdin);
    responseCode = analyze(line, &loggedIn, &tobe, &send);
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

int analyze(char *line, bool *loggedIn, bool *tobe, bool *send) {
  line[strlen(line)-1] = '\0';
  printf("|%s|\n", line);
  if(strcmp(line, "DONE") == 0) {
    return 1;
  } else if(strcmp(line, "HELP") == 0) {
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
    return 0;
  } else if(strcmp(line, "LET-ME-IN") == 0) {
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
    return 0;
  }

  char *split = strtok(line, " ");
  //TODO fix tobe, set it to false after NAME and not TOBE command, and same to bool send
  if(*loggedIn) {
    if(strcmp(split, "USER") == 0 || strcmp(split, "ACCT") == 0 || strcmp(split, "PASS") == 0) {
      printf("Already logged in.\n");
      printf("For help use HELP.\n");
      return 0;
    } else if(strcmp(split, "LIST") == 0) {

      split = strtok(NULL, " ");

      if(strcmp(split, "F") == 0 || strcmp(split, "V") == 0) {
        return 0;
      }

      return -1;
    } else if(strcmp(split, "TYPE") == 0) {
      split = strtok(NULL, " ");

      if(strcmp(split, "A") == 0 || strcmp(split, "B") == 0 || strcmp(split, "C") == 0) {
        return 0;
      }
      return -1;
    } else if(strcmp(split, "CDIR") == 0) {
      split = strtok(NULL, " ");

      if(split != NULL) {
        return -1;
      }
      return 0;
    } else if(strcmp(split, "KILL") == 0) {
      split = strtok(NULL, " ");

      if(split != NULL) {
        return -1;
      }
      return 0;
    } else if(strcmp(split, "NAME") == 0) {
      split = strtok(NULL, " ");

      if(split != NULL) {
        return -1;
      }
      *tobe = true;
      return 0;
    } else if(strcmp(split, "TOBE") == 0 && *tobe) {
      split = strtok(NULL, " ");

      if(split != NULL) {
        return -1;
      }
      *tobe = false;
      return 0;
    } else if(strcmp(split, "RETR") == 0) {
      split = strtok(NULL, " ");

      if(split != NULL) {
        return -1;
      }
      *send = true;
      return 0;
    } else if(strcmp(split, "SEND") == 0 && *send) {
      split = strtok(NULL, " ");
      *send = false;
      if(split == NULL) {
        return 0;
      }
      return -1;
    } else if(strcmp(split, "STOP") == 0 && *send) {
      split = strtok(NULL, " ");
      *send = false;
      if(split == NULL) {
        return 0;
      }
      return -1;
    } else if(strcmp(split, "STOR") == 0) {
      split = strtok(NULL, " ");

      if(strcmp(split, "NEW") == 0 || strcmp(split, "OLD") == 0 || strcmp(split, "APP") == 0 ) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          return -1;
        }
        return 0;
      }
      return -1;
    } else if(strcmp(split, "SIZE") == 0) {
      split = strtok(NULL, " ");
      char *c;
      int number = (int) strtol(split, &c, 10);
      number = number;
      if(c != NULL) {
        return -1;
      }
      return 0;
    } else {
      return -1;
    }
  } else {
    if(strcmp(split, "USER") == 0 || strcmp(split, "ACCT") == 0 || strcmp(split, "PASS") == 0) {
      split = strtok(NULL, " ");
      split = strtok(NULL, " ");
      if(split != NULL) {
        printf("Too many args.\n");
        return 0;
      }

      printf("OKAY\n");
      *loggedIn = true;
      return 0;
    } else {
      printf("First log in.\n");
      printf("Use LET-ME-IN to get help with log in.\n");
      printf("More information is in the documentation.\n");
      return 0;
    }
  }

  return -1;
}
