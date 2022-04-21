#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "serverConfig.h"
#include "auth.h"

int argumentHandler(int argc, char *argv[], ServerConfig *serverConfig);
void printUsage();
int analyze(char *line, User **user, ServerConfig *serverConfig);

int main(int argc, char *argv[]) {
  printf("hello from server\n");

  ServerConfig *serverConfig = malloc(sizeof(ServerConfig));
  initConfig(serverConfig);

  User *user = malloc(sizeof(User));
  initUser(user);
  char server_message[2000], client_message[2000];
  int socket_desc, client_sock;
  socklen_t *client_size;
  struct sockaddr_in server_addr, client_addr;

  int rc = argumentHandler(argc, argv, serverConfig);
  printf("%d\n", rc);
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

  char *line = malloc(sizeof(char) * 1000);
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
      printf("Couldn't bind to the port\n");
      return -1;
  }
  printf("Done with binding\n");

  if(listen(socket_desc, 1) < 0){
      printf("Error while listening\n");
      return -1;
  }
  printf("\nListening for incoming connections.....\n");

  client_size = (socklen_t *) sizeof(client_addr);
  client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, client_size);

    if (client_sock < 0){
        printf("Can't accept\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Receive client's message:
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return -1;
    }
    printf("Msg from client: %s\n", client_message);

    // Respond to client:
    strcpy(server_message, "This is the server's message.");

    if (send(client_sock, server_message, strlen(server_message), 0) < 0){
        printf("Can't send\n");
        return -1;
    }

    // Closing the socket:
    close(client_sock);
    close(socket_desc);

    return 0;


  while(true) {
    fgets(line, 1000, stdin);
    rc = analyze(line, &user, serverConfig);
    if(rc == 1) {
      break;
    }
    debugPrintUser(&user);
  }

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

  /*char *line = malloc(sizeof(char) * 110);
  int responseCode;*/
  /*while(true) {
    fgets(line, 100, stdin);
    responseCode = analyze(line, &loggedIn, &tobe, &send);
    if(responseCode == 1) {
      break;
    } else if(responseCode == -1) {
      printf("Not a valid command.\n");
      printf("Use HELP for help.\n");
      printf("Use LET-ME-IN for help with logging in.\n");
    }
  }*/

  free(line);
  destroyConfig(&serverConfig);
  destroyUser(&user);
  free(user);
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

int analyze(char *line, User **user, ServerConfig *serverConfig) {
  line[strlen(line)-1] = '\0';
  printf("|%s|\n", line);
  if(strcmp(line, "DONE") == 0) {
    return 1;
  }
  char *split = strtok(line, " ");
  if((*user)->loggedIn) {

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
            //send no password needed
            printf("logged in\n");
            return 0;
          } else {
            printf("bad validation\n");
            return 0;
          }

        } else {
          printf("now send password\n");
          return 0;
          //send +Valid user
        }
      }
      printf("bad user id\n");
      //send -Invalid
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
            printf("loged in\n");
            return 0;
            //send no password needed
          } else {
            printf("bad pasword\n");
            return 0;
          }

        } else {
          printf("now send password\n");
          return 0;
          //send +Valid user
        }
        return 0;
      }
      printf("bad user ID\n");
      //send -Invalid
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
            printf("logged in\n");
            return 0;
            //logged in
          } else {
            printf("Bad password\n");
            return 0;
            // bad password
          }
        }
        printf("Good but send user ID\n");
        return 0;
        //good but send user id
        return 0;
      }

      printf("bad password\n");
      // bad password
      return 0;
    } else {
      printf("First log in.\n");
    }

  }
  return 0;
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
