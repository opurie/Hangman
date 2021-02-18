#include <pthread.h>
#include <stdbool.h>
#include "config.h"
#include "signals.h"

#ifndef SERVER_H
#define SERVER_H
//int inRoom: 0 - no, 1 - creator, 2 - player
struct USER{
    char nick[MAX_NICK_LENGTH];
    int points;
    int mistakes;
    int inRoom;
};
struct USERS
{
    int registeredUsersCount;
    struct USER *players;
    pthread_mutex_t mutex;
};
struct ROOM{
    int playersIn;//max 4
    int pin;// 4 digits number
    int mistakes; // 10 - 2 players, 7 - 3 players, 5 - 4 players
    char players[MAX_PLAYERS][MAX_NICK_LENGTH];
    char creator[MAX_NICK_LENGTH];
    char name[MAX_ROOM_NAME];
    char *secretWord;//juicy word to guess
    char *InsertedLetters;
    pthread_mutex_t mutex;
};
struct ROOMS{
    int roomsCount;
    struct ROOM *rooms;
    pthread_mutex_t mutex;
};

struct THREAD_DATA{
    struct USERS *users;
    struct ROOMS *rooms;
    int fd;
};


void handle_error(int exitCode);
void writeToClient(int clientSocket, const char *message);
//Add new user. 0 - no problems, 1 - change name, 2 - limit of users is reached
int login(struct USERS * users, char * nick);
//delete user. 0 - no problems, 1 - user in game, 2 - xd
int logout(struct USERS *users, char *nick);

void initUsers(struct USERS *users);
//0-no problems, 1 - change name, 2-limit of rooms, 3 - unexpected!!!!!
int createRoom(struct ROOMS *rooms,struct USERS * users, char *roomName, char *nick, char *secretWord, int pin);

void initRooms(struct ROOMS *rooms);
//0 - no problems, 1 - room limit is reached, 2 - wrong pin, 3 - o kurwa xD
int joinRoom(struct ROOMS *rooms, struct USERS * users, char *nick, char *roomName, int pin);
//leave room or kick player 
bool leaveRoom(struct ROOMS *rooms, struct USERS * users, char *nick, char *roomName);
//delete room, nick - creator's nick
bool deleteRoom(struct ROOMS *rooms, struct USERS * users, char *nick, char *roomName);

char **returnPlayers(struct USERS *users);

char **returnRooms(struct ROOMS *rooms);

char **returnPlayersInRoom(struct ROOMS *rooms, char *roomName);
/*-1 if the letter is incorrect
0 if there was the bug and player picked letter that was already used
1 if the letter is correct*/
int pointsForLetter(char *SecretWord, char *InsertedLetters, char letter);
/*SecredWord = "BANANA IS GOOD" 
InsertedLetters = "AD" 
returns "_A_A_A-__-___D"
*/
char *returnSecretWord(char *SecretWord, char *InsertedLetters);

int stringToInt(char *pin);

void game();

#endif