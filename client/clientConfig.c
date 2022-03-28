#include <malloc.h>
#include <string.h>

#include "clientConfig.h"

void initConfig(ClientConfig *clientConfig) {
  clientConfig->workingDirectory = malloc(sizeof(char) * 1000);
  clientConfig->ip = malloc(sizeof(char) * 20);

  clientConfig->port = 115;
}

void destroyConfig(ClientConfig **clientConfig) {
  free((*clientConfig)->workingDirectory);
  free((*clientConfig)->ip);

  (*clientConfig)->workingDirectory = NULL;
  (*clientConfig)->ip = NULL;
}

void debugPrintConfig(ClientConfig **clientConfig) {
  printf("Port: %d\n", (*clientConfig)->port);
  printf("IP address: %s\n", (*clientConfig)->ip);
  printf("Working directory: %s\n", (*clientConfig)->workingDirectory);
}
