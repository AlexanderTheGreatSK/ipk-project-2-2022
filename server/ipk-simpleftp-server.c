#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>

#define MAX 80
#define PORT 115
#define SA struct sockaddr

#include "serverConfig.h"
#include "auth.h"

int argumentHandler(int argc, char *argv[], ServerConfig *serverConfig);
void printUsage();
int analyze(char *line, User **user, ServerConfig *serverConfig, char *responseMessage, char *currentDirectory, Stash *stash);
bool validPath(char* currDir ,char *path);

void func(int connfd)
{
    char buff[MAX];
    int n;
    // infinite loop for chat
    for (;;) {
        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAX);
        n = 0;
        // copy server message in the buffer
        while ((buff[n++] = getchar()) != '\n')
            ;

        // and send that buffer to client
        write(connfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}

int main(int argc, char *argv[]) {
  printf("hello from server\n");
  char *currentDirectory = malloc(sizeof(char) * 1000);
  ServerConfig *serverConfig = malloc(sizeof(ServerConfig));
  Stash *stash = malloc(sizeof(Stash));
  initConfig(serverConfig);
  initStash(stash);

  User *user = malloc(sizeof(User));
  initUser(user);

  int rc = argumentHandler(argc, argv, serverConfig);
  strcpy(currentDirectory, serverConfig->workingDirectory);
  if(rc == 1) {
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
  if(chechUserpassPath(serverConfig->passwordFile) == false) {
    printf("File does not exist.\n");
    return 1;
  }

    int sockfd, connfd, len;
    struct sockaddr_in6 servaddr, cli;
    int flag = 1;
    int flag_off = 0;

    // socket create and verification
    sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag)) < 0)
      fprintf(stderr, "ERROR setting REUSE socket option") ;

    bzero((char *) &servaddr, sizeof(servaddr));

    if((setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag)) == -1)){
        fprintf(stderr, "setsockopt failed (reuseaddr)\n");
    }

    if ((setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, "enp7s0", strlen("enp7s0"))) == -1) {
        fprintf(stderr, "setsockopt failed (INTERFACE)\n");
    }

  if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &flag_off, sizeof(flag_off)) == -1){
        fprintf(stderr,"setsockopt failed (v6only)\n");
    }

    //bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(PORT);
    servaddr.sin6_scope_id = 0;
    servaddr.sin6_flowinfo = 0;

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, SOMAXCONN)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    /*connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");*/

    // Function for chatting between client and server
    //func(connfd);

    char *line = malloc(sizeof(char) * 110);
    char *responseMessage = malloc(sizeof(char) * 1000);
    int responseCode;

    while(true) {
        fgets(line, 100, stdin);
        responseCode = analyze(line, &user, serverConfig, responseMessage, currentDirectory, stash);
        if(responseCode == 1) {
            break;
        } else if(responseCode == -1) {
            printf("Not a valid command.\n");
            printf("Use HELP for help.\n");
            printf("Use LET-ME-IN for help with logging in.\n");
        }
        if(responseMessage != NULL) {
            printf("Response message: |%s|\n", responseMessage);
        }
    }


    // After chatting close the socket
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



  /*while(true) {
    fgets(line, 1000, stdin);
    rc = analyze(line, &user, serverConfig);
    if(rc == 1) {
      break;
    }
    debugPrintUser(&user);
  }*/

  /*char *name = "kk";

  if(usernameAuth(name, serverConfig->passwordFile)) {
    printf("%s is in file.\n", name);
  } else {
    printf("%s is not in file.\n", name);
  }

  char *pass = "heslo";
  if(passwordAuth(pass, serverConfig->passwordFile)) {
    printf("%s is in file.\n", pass);
  } else {
    printf("%s is not in file.\n", pass);
  }*/

/*
  free(line);
  destroyConfig(&serverConfig);
  destroyUser(&user);
  free(user);
  free(serverConfig);
  return 0;*/
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

int analyze(char *line, User **user, ServerConfig *serverConfig, char *responseMessage, char *currentDirectory, Stash *stash) {
  strcpy(responseMessage, "\0");
  printf("CURR WD:|%s|\n", currentDirectory);
  line[strlen(line)-1] = '\0';
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
        char buff[100];
        while (fgets(buff, 100, fp) != NULL) {
          strcat(responseMessage, buff);
        }
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
        stash->operation = 1;

        strcpy(responseMessage, "+file exists");
        return 0;

      } else if(strcmp(split, "TOBE") == 0) {
        split = strtok(NULL, " ");

        if(split == NULL) {
          strcpy(responseMessage, "-bad arguments of TOBE: expected path");
          return 0;
        }

        if(stash->operation != 1) {
          strcpy(responseMessage, "-file wasn't renamed because you didn't specified one with NAME");
          return 0;
        }

        tu si zober stash a premenuje ten file na novy
      }


  } else {
    if(strcmp(split, "USER") == 0) {
      split = strtok(NULL, " ");
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
