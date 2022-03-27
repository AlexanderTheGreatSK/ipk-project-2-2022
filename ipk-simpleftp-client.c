#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "clientConfig.h"

int argumentHandler(int argc, char *argv[], ClientConfig *clientConfig);
void printUsage();

int main(int argc, char **argv) {
  printf("hello from client\n");

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

  debugPrintConfig(&clientConfig);
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
  printf("-h server IP - required parameter\n");
  printf("-p port - if not provided, default value is 115\n");
  printf("-f path to working directory - required parameter\n\n");
  printf("Description:\n");
  printf("This server works just with ipk-simpleftp-server\n\n");
  printf("Author: Alexander Okrucký [xokruc00]\n");

}
