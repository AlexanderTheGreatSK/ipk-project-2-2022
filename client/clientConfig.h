#ifndef IPK_PROJECT_2_2022_CLIENTCONFIG_H
#define IPK_PROJECT_2_2022_CLIENTCONFIG_H

typedef struct {
  char* port;
  char *ip;
  char *workingDirectory;
  int operation;
} ClientConfig;

typedef struct {
  int operation;
  int length;
  char *stash;
  char *typeStash;
} Stash;

void initConfig(ClientConfig *clientConfig);
void initStash(Stash *stash);

void destroyConfig(ClientConfig **clientConfig);
void destroyStash(Stash **stash);

void debugPrintConfig(ClientConfig **clientConfig);
void debugPrintStash(Stash **stash);
#endif //IPK_PROJECT_2_2022_CLIENTCONFIG_H
