#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "serverConfig.h"

int argumentHandler(int argc, char *argv[], ServerConfig *serverConfig);
void printUsage();

int main(int argc, char *argv[]) {
  printf("hello from server\n");

  ServerConfig *serverConfig = malloc(sizeof(ServerConfig));
  initConfig(serverConfig);

  int rc = argumentHandler(argc, argv, serverConfig);

  if( rc == 1) {
    printf("Bad arguments.\n");
    printf("Use parameter -h to get help.\n");
    destroyConfig(&serverConfig);
    free(serverConfig);
    return 1;
  } else if(rc == 2) {
    destroyConfig(&serverConfig);
    free(serverConfig);
    return 0;
  }

  destroyConfig(&serverConfig);
  free(serverConfig);
  return 0;
}

int argumentHandler(int argc, char *argv[], ServerConfig *serverConfig) {
  int option;
  char *p;

  while((option = getopt(argc, argv, "i:p:u:f:h")) != -1) {
    switch(option) {
      case 'i':
        strcpy(serverConfig->interface, optarg);
        break;
      case 'p':
        serverConfig->port = (int) strtol(optarg, &p, 10);
        break;
      case 'u':
        strcpy(serverConfig->passwordFile, optarg);
        break;
      case 'f':
        strcpy(serverConfig->workingDirectory, optarg);
        break;
      case 'h':
        printUsage();
        return 2;
        break;
      default:
        return 1;
        break;
    }
  }

  if(strlen(serverConfig->passwordFile) == 0 || strlen(serverConfig->workingDirectory) == 0) {
    return 1;
  }

  return 0;
}

void printUsage() {
  printf("Simple File Transfer Protocol - server\n");
  printf("Usage:\n");
  printf("sudo ./ipk-simpleftp-server {-i interface} {-p port} [-u path to user file] [-f path to working directory]\n");
  printf("-i interface - if not provided, default value is eth0\n");
  printf("-p port - if not provided, default value is 115\n");
  printf("-u path to user file - required parameter\n");
  printf("-f path to working directory - required parameter\n\n");
  printf("Description:\n");
  printf("Please use super user privileges.\n");
  printf("This server works just with ipk-simpleftp-client\n\n");
  printf("Author: Alexander Okrucký [xokruc00]\n");

}
