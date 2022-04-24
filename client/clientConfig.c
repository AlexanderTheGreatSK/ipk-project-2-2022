#include <malloc.h>
#include <string.h>

#include "clientConfig.h"

void initConfig(ClientConfig *clientConfig) {
  clientConfig->workingDirectory = malloc(sizeof(char) * 1000);
  clientConfig->ip = malloc(sizeof(char) * 50);
  clientConfig->port = malloc(sizeof(char) * 10);

  clientConfig->operation = -1;
  strcpy(clientConfig->port, "115");
}

void destroyConfig(ClientConfig **clientConfig) {
  free((*clientConfig)->workingDirectory);
  free((*clientConfig)->ip);
  free((*clientConfig)->port);

  (*clientConfig)->operation = -1;
  (*clientConfig)->workingDirectory = NULL;
  (*clientConfig)->ip = NULL;
  (*clientConfig)->port = NULL;
}

void debugPrintConfig(ClientConfig **clientConfig) {
  printf("Port: %s\n", (*clientConfig)->port);
  printf("IP address: %s\n", (*clientConfig)->ip);
  printf("Operation: %d\n", (*clientConfig)->operation);
  printf("Working directory: %s\n", (*clientConfig)->workingDirectory);
}

void initStash(Stash *stash) {
  stash->stash = malloc(sizeof(char) * 1000);
  stash->typeStash = malloc(sizeof(char) * 1000);
  stash->operation = -1;
  stash->length = -1;
  strcpy(stash->stash, "");
  strcpy(stash->typeStash, "");
}

void destroyStash(Stash **stash) {
  free((*stash)->stash);
  free((*stash)->typeStash);

  (*stash)->stash = NULL;
  (*stash)->typeStash = NULL;
  (*stash)->operation = -1;
  (*stash)->length = -1;
}

void debugPrintStash(Stash **stash) {

  printf("Stash: %s\n", (*stash)->stash);
  printf("Type stash: %s\n", (*stash)->typeStash);
  printf("Operation: %d\n", (*stash)->operation);
  printf("Length: %d\n", (*stash)->length);

}
