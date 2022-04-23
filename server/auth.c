#include <malloc.h>
#include <string.h>

#include "auth.h"

bool usernameAuth(char *name, char *userpassPath) {
  FILE *fp;

  fp = fopen(userpassPath, "r");
  char *line = malloc(sizeof(char) * 1000);

  while(fgets(line, 1000, fp) != NULL) {
    if(line[strlen(line)-1] == '\n') {
      line[strlen(line)-1] = '\0';
    }
    line = strtok(line, ":");
    if(strcmp(line, name) == 0) {
      free(line);
      fclose(fp);
      return true;
    }
  }

  free(line);
  fclose(fp);
  return false;
}

bool passwordAuth(char *password, char *userpassPath) {
  FILE *fp;

  fp = fopen(userpassPath, "r");
  char *line = malloc(sizeof(char) * 1000);
  char *split;

  while(fgets(line, 1000, fp) != NULL) {
    if(line[strlen(line)-1] == '\n') {
      line[strlen(line)-1] = '\0';
    }
    split = strtok(line, ":");
    split = strtok(NULL, "");
    if(strcmp(split, password) == 0) {
      free(line);
      fclose(fp);
      return true;
    }
  }

  free(line);
  fclose(fp);
  return false;
}

bool namePasswordValidation(char *name, char *password, char *userpassPath) {
  FILE *fp;

  fp = fopen(userpassPath, "r");
  char *line = malloc(sizeof(char) * 1000);
  char *split;

  while(fgets(line, 1000, fp) != NULL) {
    if(line[strlen(line)-1] == '\n') {
      line[strlen(line)-1] = '\0';
    }
    split = strtok(line, ":");
    if(strcmp(split, name) == 0) {

      split = strtok(NULL, "");

      if(strcmp(split, password) == 0) {
        free(line);
        fclose(fp);
        return true;
      } else {
        free(line);
        fclose(fp);
        return false;
      }
    }
  }

  free(line);
  fclose(fp);
  return false;
}

bool chechUserpassPath(char *userpassPath) {
  FILE *fp;

  fp = fopen(userpassPath, "r");


  if(!fp) {
    return false;
  } else {
    fclose(fp);
    return true;
  }
}

void initUser(User *user) {
  user->name = malloc(sizeof(char) * 1000);
  user->password = malloc(sizeof(char) * 1000);
  user->loggedIn = false;
}

void destroyUser(User **user) {
  free((*user)->name);
  free((*user)->password);

  (*user)->name = NULL;
  (*user)->password = NULL;
  (*user)->loggedIn = NULL;
}

void debugPrintUser(User **user) {
  printf("User name: %s\n", (*user)->name);
  printf("User password: %s\n", (*user)->password);
  printf("User logged in: %s\n", ((*user)->loggedIn) ? "YES" : "NO");
}