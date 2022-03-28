#ifndef IPK_PROJECT_2_2022_SERVERCONFIG_H
#define IPK_PROJECT_2_2022_SERVERCONFIG_H

/*
 * Structure for saving program arguments and using them as server config
 *
 * @param port                -> listening port
 * @param interface           -> interface for communication
 * @param passwordFile        -> path to password file
 * @param workingDirectory    -> path to working directory
 * */
typedef struct {
int port;
char *interface;
char *passwordFile;
char *workingDirectory;
} ServerConfig;

/*
 * Initialize server config data structure
 *
 * @param ServerConfig -> given ServerConfig data structure
 * */
void initConfig(ServerConfig *serverConfig);

/*
 * Destroy server config data structure
 *
 * @param ServerConfig -> given ServerConfig data structure
 * */
void destroyConfig(ServerConfig **serverConfig);

void debugPrintConfig(ServerConfig **serverConfig);

#endif //IPK_PROJECT_2_2022_SERVERCONFIG_H
