#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 80
#define PORT 115
#define SA struct sockaddr

#include "serverConfig.h"
#include "auth.h"

int argumentHandler(int argc, char *argv[], ServerConfig *serverConfig);
void printUsage();
int analyze(char *line, User **user, ServerConfig *serverConfig);

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
    
const char *opt;
opt = "enp0s3";
int len6 = 6;


    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag)) < 0)
      fprintf(stderr, "ERROR setting REUSE socket option") ;

    bzero((char *) &servaddr, sizeof(servaddr));

    /*if((setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag)) == -1)){
        fprintf(stderr, "setsockopt failed (reuseaddr)\n");
    }*/

    /*if ((setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *) &opt, strlen(opt))) == -1) {
        fprintf(stderr, "setsockopt failed (INTERFACE)\n");
    }*/

    /*if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &flag_off, sizeof(flag_off)) == -1){
        fprintf(stderr,"setsockopt failed (v6only)\n");
    }*/



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
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);
    return 0;

  /*ServerConfig *serverConfig = malloc(sizeof(ServerConfig));
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
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

    if (client_size == 0){
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
