#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>

#define PORT 115
#define NONE -1
#define RENAME 1
#define SEND 2
#define RECEIVE_NEW 3
#define RECEIVE_OLD 4
#define RECEIVE_APP 5
#define SA struct sockaddr

#include "serverConfig.h"
#include "auth.h"

int argumentHandler(int argc, char *argv[], ServerConfig *serverConfig);
void printUsage();
int analyze(char *line, User **user, ServerConfig *serverConfig, char *responseMessage, char *currentDirectory, Stash *stash, int sockfd);
bool validPath(char* currDir ,char *path);
void appendToFile(char *filePath, char *content);

void catchSignal(int signal) {
  if (signal == SIGINT) {
    close(PORT);
    exit(0);
  }
}

int main(int argc, char *argv[]) {
  int sockfd, connfd, len;
  signal(SIGINT, catchSignal);
  char *currentDirectory = malloc(sizeof(char) * 1000);
  ServerConfig *serverConfig = malloc(sizeof(ServerConfig));
  Stash *stash = malloc(sizeof(Stash));
  initConfig(serverConfig);
  initStash(stash);

  User *user = malloc(sizeof(User));
  initUser(user);

  int rc = argumentHandler(argc, argv, serverConfig);
  strcpy(currentDirectory, serverConfig->workingDirectory);
  if (rc == 1) {
    printf("Bad arguments.\n");
    printf("Use parameter -h to get help.\n");
    destroyConfig(&serverConfig);
    free(serverConfig);
    return 1;
  } else if (rc == 2) {
    destroyConfig(&serverConfig);
    free(serverConfig);
    return 0;
  }
  if (chechUserpassPath(serverConfig->passwordFile) == false) {
    printf("File does not exist.\n");
    return 1;
  }

  struct sockaddr_in6 servaddr, cli;
  int flagOn = 1;
  int flagOff = 0;
  int r;

  char *line = malloc(sizeof(char) * 110);
  char *responseMessage = malloc(sizeof(char) * 2000);
  int responseCode;

  while(true) {


    sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
      printf("socket creation failed...\n");
      exit(0);
    } else
      printf("Socket successfully created..\n");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flagOn, sizeof(flagOn)) < 0)
      fprintf(stderr, "ERROR setting REUSE socket option");

    bzero((char *) &servaddr, sizeof(servaddr));

    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flagOn, sizeof(flagOn)) == -1)) {
      fprintf(stderr, "setsockopt failed (reuseaddr)\n");
    }

    if ((setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, serverConfig->interface, strlen(serverConfig->interface))) ==
        -1) {
      fprintf(stderr, "setsockopt failed (INTERFACE)\n");
    }

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &flagOff, sizeof(flagOff)) == -1) {
      fprintf(stderr, "setsockopt failed (v6only)\n");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(serverConfig->port);
    servaddr.sin6_scope_id = 0;
    servaddr.sin6_flowinfo = 0;

    if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) != 0) {
      printf("socket bind failed...\n");
      exit(0);
    } else {
      printf("Socket successfully binded..\n");
    }

    if ((listen(sockfd, SOMAXCONN)) != 0) {
      printf("Listen failed...\n");
      exit(0);
    } else {
      printf("Server listening..\n");
    }

    len = sizeof(cli);
    connfd = accept(sockfd, (SA *) &cli, &len);
    if (connfd < 0) {
      printf("server accept failed...\n");
      exit(0);
    } else {
      printf("server accept the client...\n");
    }
    strcpy(responseMessage, "+welcome, SFTP service active");
    send(connfd, responseMessage, strlen(responseMessage), 0);
    strcpy(responseMessage, "");
    while (true) {

      r = recv(connfd, line, 100, 0);
      if (r < 0) {
        fprintf(stderr, "ERROR.\n");
        break;
      }

      printf("%s\n", line);

      if (r == 0) {
        break;
      }

      responseCode = analyze(line, &user, serverConfig, responseMessage, currentDirectory, stash, sockfd);
      printf("%s\n", responseMessage);
      r = send(connfd, responseMessage, strlen(responseMessage), 0);
      if (r < 0) {
        break;
      }
    }
  }
  close(sockfd);
  free(line);
  free(responseMessage);
  free(currentDirectory);
  destroyConfig(&serverConfig);
  destroyUser(&user);
  free(user);
  free(serverConfig);
  destroyStash(&stash);
  free(stash);
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

int analyze(char *line, User **user, ServerConfig *serverConfig, char *responseMessage, char *currentDirectory, Stash *stash, int sockfd) {
  strcpy(responseMessage, "\0");
    FILE *fp;
    char command[100];
  if(strcmp(line, "DONE") == 0) {
    return 1;
  }
  char *split = strtok(line, " ");
  if((*user)->loggedIn) {
      if(strcmp(split, "LIST") == 0) {
          split = strtok(NULL, " ");
          int mode = -1;

          if(split == NULL) {
            strcpy(responseMessage, "-bad arguments of LIST: expected F or V");
            return 0;
          }

          if(strcmp(split, "F") == 0) {
            mode = 1;
          } else if(strcmp(split, "V") == 0) {
            mode = 2;
          } else {
            strcpy(responseMessage, "-bad arguments of LIST: expected F or V");
            return 0;
          }
          split = strtok(NULL, " ");
          if(mode == 1) {
              if (split == NULL) {
                sprintf(command, "ls -w 1 %s", currentDirectory);
                fp = popen(command, "r");
              } else {
                if(validPath(currentDirectory,split) == false) {
                  printf("Bad path to directory.");
                  strcpy(responseMessage, "-directory or file does not exists");
                  return 0;
                }

                  sprintf(command, "ls -w 1 %s", split);
                  fp = popen(command, "r");
              }
          } else {
              if (split == NULL) {
                sprintf(command, "ls -l %s", currentDirectory);
                fp = popen(command, "r");
              } else {
                if(validPath(currentDirectory, split) == false) {
                  strcpy(responseMessage, "-bad directory path");
                  return 0;
                }
                  sprintf(command, "ls -l %s", split);
                  fp = popen(command, "r");
              }
          }
        char buff[1000];
        while (fgets(buff, 100, fp) != NULL) {
          strcat(responseMessage, buff);
        }
        responseMessage[strlen(responseMessage)-1] = '\0';
        return 0;
      } else if(strcmp(split, "CDIR") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of CDIR: expected path");
          return 0;
        }

        if(validPath(currentDirectory,split) == false) {
          strcpy(responseMessage, "-bad directory path");
          return 0;
        }

        if(split[0] ==  '/') {
          strcpy(currentDirectory, split);
          strcpy(responseMessage, "!changed working dir to ");
          strcat(responseMessage, split);
          return 0;
        } else if(strcmp(split, "..") == 0) {
          int index = strlen(currentDirectory)-2;

          while(index >= 0) {
            if(currentDirectory[index] == '/') {
              currentDirectory[index] = '\0';
              break;
            } else if(index == 0) {
              strcpy(currentDirectory, "/");
            }
            index--;
          }

          strcpy(responseMessage, "!changed working dir to ");
          strcat(responseMessage, currentDirectory);
          return 0;

        } else if(strcmp(split, ".") == 0) {
          strcpy(responseMessage, "!changed working dir to ");
          strcat(responseMessage, currentDirectory);
          return 0;
        } else {
          int len = strlen(currentDirectory);
          if(currentDirectory[len-1] != '/') {
            strcat(currentDirectory, "/");
          }

          strcat(currentDirectory, split);
          strcpy(responseMessage, "!changed working dir to ");
          strcat(responseMessage, split);
          return 0;
        }

      } else if(strcmp(split, "HOME") == 0) {
        strcpy(currentDirectory, serverConfig->workingDirectory);
        strcpy(responseMessage, "+changed to basic working directory");
        return 0;
      } else if(strcmp(split, "KILL") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of KILL: expected path");
          return 0;
        }

        if(validPath(currentDirectory, split) == false) {
          strcpy(responseMessage, "-not deleted because file does not exist");
          return 0;
        } else {

          if(split[0] ==  '/') {
            sprintf(command, "rm -r ");
            strcat(command, split);
            system(command);

            strcpy(responseMessage, "+");
            strcat(responseMessage, split);
            strcat(responseMessage, " deleted");

            return 0;
          } else {
            int len = strlen(currentDirectory);
            if(currentDirectory[len-1] != '/') {
              strcat(currentDirectory, "/");
            }

            strcpy(command, "rm -r ");
            strcat(command, currentDirectory);
            strcat(command, split);
            system(command);

            strcpy(responseMessage, "+");
            strcat(responseMessage, split);
            strcat(responseMessage, " deleted");
            return 0;
          }
        }
      } else if(strcmp(split, "NAME") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of NAME: expected path");
          return 0;
        }

        if(validPath(currentDirectory, split) == false) {
          strcpy(responseMessage, "-can't find file ");
          strcat(responseMessage, split);
          return 0;
        }

        strcpy(stash->stash, split);
        stash->operation = RENAME;

        strcpy(responseMessage, "+file exists");
        return 0;

      } else if(strcmp(split, "TOBE") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of TOBE: expected path");
          return 0;
        }

        if(stash->operation != RENAME) {
          strcpy(responseMessage, "-file wasn't renamed because you didn't specified one with NAME");
          return 0;
        }

        char *path = malloc(sizeof(char) * 100);

        strcpy(path, currentDirectory);

        if(currentDirectory[strlen(currentDirectory)-1] != '/') {
          strcat(path, "/");
        }
        printf("TU\n");
        strcat(path, stash->stash);
        strcpy(stash->typeStash, split);

        strcpy(command, "mv ");
        strcat(command, path);
        strcat(command, " ");
        printf("TU\n");
        strcpy(path, currentDirectory);

        if(currentDirectory[strlen(currentDirectory)-1] != '/') {
          strcat(path, "/");
        }
        printf("TU\n");
        strcat(path, split);
        strcat(command, path);
        printf("TU\n");
        system(command);

        strcpy(responseMessage, "+");
        printf("TU\n");
        strcat(responseMessage, stash->stash);
        printf("TU\n");
        strcat(responseMessage, "renamed to ");
        printf("TU\n");
        printf("S:%s\n", stash->typeStash);
        strcat(responseMessage, stash->typeStash);
        printf("TU\n");
        strcpy(stash->stash, "");
        stash->operation = NONE;
        printf("TU\n");
        return 0;
      } else if(strcmp(split, "TYPE") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of TYPE: expected type");
          return 0;
        }

        if(strcmp(split, "A") == 0) {
          serverConfig->type = 'A';
          strcpy(responseMessage, "+using ASCII mode");
          return 0;
        } else if(strcmp(split, "B") == 0) {
          serverConfig->type = 'B';
          strcpy(responseMessage, "+using BINARY mode");
          return 0;
        } else if(strcmp(split, "C") == 0) {
          serverConfig->type = 'B';
          strcpy(responseMessage, "+using CONTINUOUS mode");
          return 0;
        } else {
          strcpy(responseMessage, "-type not valid");
          return 0;
        }
      } else if(strcmp(split, "RETR") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of RETR: path to file");
          return 0;
        }

        if(validPath(currentDirectory, split) == false) {
          strcpy(responseMessage, "-file doesn't exist");
          return 0;
        }

        fp = fopen(split, "r");
        int size = 0;
        if(fp) {
          fseek(fp, 0L, SEEK_END);
          size = ftell(fp);
          rewind(fp);
          fclose(fp);
        } else {
          strcpy(responseMessage, "-during reading error happened, try again");
          return 0;
        }

        stash->operation = SEND;
        strcpy(stash->stash, split);

        sprintf(responseMessage, "%d", size);
        return 0;
      } else if(strcmp(split, "SEND") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of SEND: path to file");
          return 0;
        }

        if(stash->operation != SEND) {
          strcpy(responseMessage, "-not specified what to SEND, use RETR");
          return 0;
        }

        int b;
        char sendBuffer[100];
        strcpy(command, currentDirectory);
        if (currentDirectory[strlen(currentDirectory)-1] != '/') {
          strcat(command, "/");
        }
        strcat(command, split);
        printf("%s\n", command);

        FILE *fpp = fopen(command, "rb");
        if(fpp == NULL){
          fprintf(stderr, "File\n");
          return 2;
        }

        while( (b = fread(sendBuffer, 1, sizeof(sendBuffer), fp))>0 ){
          send(sockfd, sendBuffer, b, 0);
        }

        stash->operation = NONE;
        strcpy(stash->stash, "");
        strcpy(command, "");
        return 0;
      } else if(strcmp(split, "STOP") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of SEND: path to file");
          return 0;
        }

        if(stash->operation != SEND) {
          strcpy(responseMessage, "-not specified what to STOP, use RETR");
          return 0;
        }

        stash->operation = NONE;
        strcpy(stash->stash, "");
        strcpy(responseMessage, "+ok, RETR aborted");
        return 0;
      } else if(strcmp(split, "STOR") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of STOR: missing type");
          return 0;
        }

        if(strcmp(split, "NEW") == 0) {
          stash->operation = RECEIVE_NEW;
        } else if(strcmp(split, "OLD") == 0) {
          stash->operation = RECEIVE_OLD;
        } else if(strcmp(split, "APP") == 0) {
          stash->operation = RECEIVE_APP;
        } else {
          strcpy(responseMessage, "-bad arguments of STOR: bad type");
          return 0;
        }

        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of STOR: missing path");
          return 0;
        }

        strcpy(stash->stash, split);

        if(stash->operation == RECEIVE_NEW) {
          if(validPath(currentDirectory, split)) {
            strcpy(responseMessage, "+file exists, will create new generation of file");
          } else {
            strcpy(responseMessage, "+file does not exist, will create new file");
          }
        } else if(stash->operation == RECEIVE_OLD) {
          if(validPath(currentDirectory, split)) {
            strcpy(responseMessage, "+will write over old file");
          } else {
            strcpy(responseMessage, "+will create new file");
          }
        } else if(stash->operation == RECEIVE_APP) {
          if(validPath(currentDirectory, split)) {
            strcpy(responseMessage, "+will append to file");
          } else {
            strcpy(responseMessage, "+will create new file");
          }
        }
        return 0;
      } else if(strcmp(split, "SIZE") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of SIZE: missing size");
          return 0;
        }

        //TODO malloc char*size
        //call function to do that

        strcpy(responseMessage, "+saved ");
        strcat(responseMessage, stash->stash);

        stash->operation = NONE;
        strcpy(stash->stash, "");

        return 0;
      }


  } else {
    if(strcmp(split, "USER") == 0) {
      split = strtok(NULL, " ");
      printf("%s\n", split);
      if(split == NULL) {
        return -1;
      }

      if(usernameAuth(split, serverConfig->passwordFile)) {
        strcpy((*user)->name, split);

        if(strlen((*user)->password) > 1) {

          if(namePasswordValidation((*user)->name, (*user)->password, serverConfig->passwordFile)) {
            (*user)->loggedIn = true;
            strcpy(responseMessage, "! ");
            strcat(responseMessage, split);
            strcat(responseMessage, " logged in");
            return 0;
          } else {
            strcpy(responseMessage, "-invalid user-id");
            return 0;
          }

        } else {
          strcpy(responseMessage, "+user-id valid");
          return 0;
        }
      }
      strcpy(responseMessage, "-invalid user-id");
      return 0;
    } else if(strcmp(split, "ACCT") == 0) {
      split = strtok(NULL, " ");
      if(split == NULL) {
        return -1;
      }

      if(usernameAuth(split, serverConfig->passwordFile)) {
        strcpy((*user)->name, split);

        if(strlen((*user)->password) > 1) {

          if(namePasswordValidation((*user)->name, (*user)->password, serverConfig->passwordFile)) {
            (*user)->loggedIn = true;
            strcpy(responseMessage, "!account valid");
            return 0;
          } else {
            strcpy(responseMessage, "-invalid account");
            return 0;
          }

        } else {
          printf("now send password\n");
          strcpy(responseMessage, "+account valid");
          return 0;
        }
      }
      strcpy(responseMessage, "-invalid account");
      return 0;

    } else if(strcmp(split, "PASS") == 0) {
      split = strtok(NULL, " ");
      if(split == NULL) {
        return -1;
      }

      if(passwordAuth(split, serverConfig->passwordFile)) {
        strcpy((*user)->password, split);

        if(strlen((*user)->name) > 1) {

          if(namePasswordValidation((*user)->name, (*user)->password, serverConfig->passwordFile)) {
            (*user)->loggedIn = true;
            strcpy(responseMessage, "!logged in");
            return 0;
          } else {
            strcpy(responseMessage, "-wrong password");
            return 0;
          }
        }
        strcpy(responseMessage, "+send account");
        return 0;
      }
      strcpy(responseMessage, "-wrong password");
      return 0;
    } else {
      strcpy(responseMessage, "-first log in");
    }

  }
  return 0;
}

void appendToFile(char *filePath, char *content) {
  FILE *fp;

  fp = fopen(filePath, "a");
  fputs(content, fp);
  fclose(fp);
}

bool validPath(char* currDir ,char *path) {
  char buff[100];
  int len;
  FILE *f;
  DIR *d;

  len = strlen(currDir);
  strcpy(buff, currDir);

  if (path[0] != '/') {
    if(currDir[len - 1] == '/') {
      strcat(buff, path);
    } else {
      strcat(buff, "/");
      strcat(buff, path);
    }
  } else {
    strcpy(buff, path);
  }

  f = fopen(buff, "r");
  if(f) {
    fclose(f);
    return true;
  } else {
    d = opendir(buff);
    if(d) {
      closedir(d);
      return true;
    } else {
      return false;
    }

  }
}

//Printing zone
//-----------------------------------------------------------------------------------
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
