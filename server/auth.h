#ifndef IPK_PROJECT_2_2022_AUTH_H
#define IPK_PROJECT_2_2022_AUTH_H

#include <stdbool.h>

typedef struct {
  char *name;
  char *password;
  bool loggedIn;
} User;


bool usernameAuth(char *name, char *userpassPath);

bool passwordAuth(char *password, char *userpassPath);

bool chechUserpassPath(char *userpassPath);

bool namePasswordValidation(char *name, char *password, char *userpassPath);

void initUser(User *user);

void destroyUser(User **user);

void debugPrintUser(User **user);

#endif //IPK_PROJECT_2_2022_AUTH_H
