#ifndef IPK_PROJECT_2_2022_CLIENTCONFIG_H
#define IPK_PROJECT_2_2022_CLIENTCONFIG_H

typedef struct {
  int port;
  char *ip;
  char *workingDirectory;
} ClientConfig;

void initConfig(ClientConfig *clientConfig);

void destroyConfig(ClientConfig **clientConfig);

void debugPrintConfig(ClientConfig **clientConfig);

#endif //IPK_PROJECT_2_2022_CLIENTCONFIG_H
