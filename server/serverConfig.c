#include <malloc.h>
#include <string.h>

#include "serverConfig.h"

void initConfig(ServerConfig *serverConfig) {
  serverConfig->workingDirectory = malloc(sizeof(char) * 1000);
  serverConfig->passwordFile = malloc(sizeof(char) * 1000);
  serverConfig->interface = malloc(sizeof(char) * 1000);

  serverConfig->port = 115;
  strcpy(serverConfig->interface, "eth0");
  strcpy(serverConfig->passwordFile, "");
  strcpy(serverConfig->workingDirectory, "");
}

void destroyConfig(ServerConfig **serverConfig) {
  free((*serverConfig)->workingDirectory);
  free((*serverConfig)->passwordFile);
  free((*serverConfig)->interface);

  (*serverConfig)->workingDirectory = NULL;
  (*serverConfig)->passwordFile = NULL;
  (*serverConfig)->interface = NULL;
}

void debugPrintConfig(ServerConfig **serverConfig) {

  printf("Port: %d\n", (*serverConfig)->port);
  printf("Interface: %s\n", (*serverConfig)->interface);
  printf("Passwords: %s\n", (*serverConfig)->passwordFile);
  printf("Working directory: %s\n", (*serverConfig)->workingDirectory);
}

void initStash(Stash *stash) {
  stash->stash = malloc(sizeof(char) * 1000);
  stash->operation = -1;
  strcpy(stash->stash, "");
}

void destroyStash(Stash **stash) {
  free((*stash)->stash);

  (*stash)->stash = NULL;
  (*stash)->operation = -1;
}

void debugPrintStash(Stash **stash) {

  printf("Stash: %s\n", (*stash)->stash);
  printf("Operation: %d\n", (*stash)->operation);

}