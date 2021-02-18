#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

#include "server.h"
#include "config.h"


#ifndef MAIN_H
#define MAIN_H

void *worker(void *input);

bool loginToServer(int connfd, struct USERS *users, char *nick);

bool lobby(int connfd, struct USERS *users, struct ROOMS *rooms, char *nick, char *roomName);

//for creator
bool HandleRoom(int connfd, struct USERS *users, struct ROOMS *rooms);

bool getInRoom(int connfd, struct USERS *users, struct ROOMS *rooms);

void newUser(int connfd, struct USERS *users, struct ROOMS *rooms);

int main(int argc, char *argv[]);

#endif