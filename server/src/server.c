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
#include "err.h"

void handle_error(int exitCode){
    if(exitCode<0){
        fprintf(stderr, "Error: %s\n", strerror( errno ));
        exit(1);
    }
}
void replaceDelimiter(char *txt){
    char rep = '\t';
    char orig = '\n';
    char *ix = txt;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
    }
}
void writeToClient(int clientSocket, const char *message){
    handle_error(write(clientSocket, message, strlen(message)));
}

int login(struct USERS *users, char *nick){
    pthread_mutex_lock(&(users->mutex));
    //Check if users limit is reached
    if(users->registeredUsersCount == MAX_USR_COUNT){
        pthread_mutex_unlock(&(users->mutex));
        return 2;
    }
    //Check if nick is allowed to use
    for(int i=0; i< users->registeredUsersCount;i++){
        if(strcmp(users->players[i].nick, nick)==0){
            pthread_mutex_unlock(&(users->mutex));
            return 1;
        }
    }
    //Insert new player
    struct USER *player = malloc(sizeof(struct USER));
    users->players = realloc(users->players, (users->registeredUsersCount+1)*sizeof(struct USER));
    player->inRoom=0;
    player->mistakes=0;
    player->points=0;
    strncpy(player->nick, nick, MAX_NICK_LENGTH);
    users->players[users->registeredUsersCount]=*player;
    users->registeredUsersCount++;
    pthread_mutex_unlock(&(users->mutex));
    printf("Created player %s\n",nick);
    return 0;
}

int logout(struct USERS *users, char *nick){
    pthread_mutex_lock(&(users->mutex));
    struct USER *tmp = malloc(sizeof(struct USER));
    for(int i = 0; i<users->registeredUsersCount; i++){
        if(strcmp(users->players[i].nick, nick) == 0){
            if(users->players[i].inRoom > 0){
                pthread_mutex_unlock(&(users->mutex));
                return 1;
            }
            tmp = &users->players[users->registeredUsersCount-1];
            users->players[i] = *tmp;
            users->players = realloc(users->players, (users->registeredUsersCount-1)*sizeof(struct USER));
            users->registeredUsersCount--;
            break;
        }
    }
    pthread_mutex_unlock(&(users->mutex));
    return 0;
}

void initUsers(struct USERS *users){
    pthread_mutex_init(&(users->mutex), NULL);
    users->registeredUsersCount=0;
}

int createRoom(struct ROOMS *rooms, struct USERS * users, char *roomName, char *nick, char *secretWord, int pin){
    pthread_mutex_lock(&(rooms->mutex));
    struct USER *player = NULL;

    if(rooms->roomsCount == MAX_ROOMS_COUNT){
        pthread_mutex_unlock(&(rooms->mutex));
        return 2;
    }
    //check if user is real or is already in room
    pthread_mutex_lock(&(users->mutex));
    for(int i = 0; i < users->registeredUsersCount; i++){
        if((strcmp(users->players[i].nick, nick)==0 && users->players[i].inRoom > 0) || 
            (!strcmp(users->players[i].nick, nick)==0 && i == users->registeredUsersCount-1)){
            
            pthread_mutex_unlock(&(users->mutex));
            pthread_mutex_unlock(&(rooms->mutex));
            return 3;
        }
        else if (strcmp(users->players[i].nick, nick)==0 && users->players[i].inRoom == 0){
            player = &users->players[i];
        }
    }
    pthread_mutex_unlock(&(users->mutex));
    
    //Check if name is allowed to use
    for(int i=0; i < rooms->roomsCount; i++){
        if(strcmp(rooms->rooms[i].name, roomName) == 0){
            pthread_mutex_unlock(&(rooms->mutex));
            return 1;
        }
    }
    struct ROOM * room = malloc(sizeof(struct ROOM));
    rooms->rooms = realloc(rooms->rooms, (rooms->roomsCount+1)*sizeof(struct ROOM));
    int len = sizeof(&secretWord)/sizeof(secretWord[0]);
    if(len > MAX_SECRET_WORD)
        len = MAX_SECRET_WORD;
    room->secretWord = malloc(sizeof(char)*len);
    room->InsertedLetters = malloc(sizeof(char)*0);
    room->mistakes = 0;
    room->pin = pin;
    room->playersIn = 0;
    strncpy(room->name, roomName, MAX_ROOM_NAME);
    strncpy(room->creator, nick, MAX_NICK_LENGTH);
    strncpy(room->secretWord, secretWord, len);
    pthread_mutex_init(&(room->mutex),NULL);

    rooms->rooms[rooms->roomsCount] = *room;
    rooms->roomsCount++;
    player->inRoom = 1;
    printf("Created room: %s pin: %d count: %d\n", room->name, room->pin, rooms->roomsCount);
    pthread_mutex_unlock(&(rooms->mutex));
    return 0;
}

void initRooms(struct ROOMS *rooms){
    pthread_mutex_init(&(rooms->mutex), NULL);
    rooms->roomsCount = 0;
}

int joinRoom(struct ROOMS *rooms, struct USERS * users, char *nick, char *roomName, int pin){
    pthread_mutex_lock(&(rooms->mutex));
    pthread_mutex_lock(&(users->mutex));
    struct USER *user =malloc(sizeof(struct USER));
    //check if player already is in room
    for(int i=0; i < users->registeredUsersCount; i++){
        if(strcmp(users->players[i].nick, nick) == 0 && users->players[i].inRoom > 0){
            pthread_mutex_unlock(&(users->mutex));
            pthread_mutex_unlock(&(rooms->mutex));
            return 3;
        }
        else if(strcmp(users->players[i].nick, nick) == 0 && users->players[i].inRoom == 0){
            users->players[i].mistakes = 0;
            users->players[i].points = 0;
            user = &users->players[i];
            break;
        }
    }

    //check if room limit isn't reached
    for(int i = 0; i < rooms->roomsCount; i++){
        if(strcmp(rooms->rooms[i].name, roomName) == 0 && rooms->rooms[i].playersIn == MAX_PLAYERS){
            pthread_mutex_unlock(&(users->mutex));
            pthread_mutex_unlock(&(rooms->mutex));
            return 1;
        }
    }

    //join
    for(int i = 0; i < rooms->roomsCount; i++){
        if(strcmp(rooms->rooms[i].name, roomName) == 0){
            if(rooms->rooms[i].pin != pin)
                return 2;

            strncpy(rooms->rooms[i].players[rooms->rooms[i].playersIn], nick, MAX_NICK_LENGTH);
            rooms->rooms[i].playersIn++;
            if(rooms->rooms[i].playersIn == 2)
                rooms->rooms[i].mistakes = 10;
            else if(rooms->rooms[i].playersIn == 3)
                rooms->rooms[i].mistakes = 7;
            else if(rooms->rooms[i].playersIn == 4)
                rooms->rooms[i].mistakes = 5;
            break;
        }
        if(i == rooms->roomsCount-1)
            return 3;
    }
    user->inRoom=2;
    pthread_mutex_unlock(&(users->mutex));
    pthread_mutex_unlock(&(rooms->mutex));
    return 0;
}

bool leaveRoom(struct ROOMS *rooms, struct USERS * users, char *nick, char *roomName){ 
    pthread_mutex_lock(&(users->mutex));
    struct USER *player=NULL;
    //find player
    for(int i = 0; i < users->registeredUsersCount; i++){
        if(strcmp(users->players[i].nick, nick)==0){    
            player = &users->players[i];    
            break;
        }
        if(i == users->registeredUsersCount-1){    
            pthread_mutex_unlock(&(users->mutex));
            return false;
        }
    }
    //find room and kick player
    pthread_mutex_lock(&(rooms->mutex));
    for (int i = 0; i < rooms->roomsCount; i++){
        if(strcmp(rooms->rooms[i].name, roomName) == 0){
            for(int j = 0; j < rooms->rooms[i].playersIn; j++){
                if(strcmp((*player).nick, rooms->rooms[i].players[j]) == 0){
                    (*player).inRoom = 0;
                    //Sending last player to a kicked player position and decrementing a counter
                    strncpy(rooms->rooms[i].players[j], rooms->rooms[i].players[rooms->rooms[i].playersIn-1], MAX_NICK_LENGTH);
                    rooms->rooms[i].playersIn--;
                    pthread_mutex_unlock(&(users->mutex));
                    pthread_mutex_unlock(&(rooms->mutex));
                    return true;
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&(users->mutex));
    pthread_mutex_unlock(&(rooms->mutex));
    return false;
}

bool deleteRoom(struct ROOMS *rooms, struct USERS * users, char *nick, char *roomName){
    pthread_mutex_lock(&(rooms->mutex));
    pthread_mutex_lock(&(users->mutex));

    //find player and check if he is creator
    for(int i = 0; i < users->registeredUsersCount; i++){
        if(strcmp(users->players[i].nick, nick)==0 && users->players[i].inRoom == 1)    
            break;
        if(i == users->registeredUsersCount-1){    
            pthread_mutex_unlock(&(users->mutex));
            pthread_mutex_unlock(&(rooms->mutex));
            return false;
        }
    }
    //kick them out
    for(int i = 0; i< rooms->roomsCount; i++){
        if(strcmp(rooms->rooms[i].name, roomName) == 0){
            for(int j = 0; j < rooms->rooms[i].playersIn; j++){
                for( int k = 0; k<users->registeredUsersCount; k++){
                    if(strcmp(users->players[k].nick, rooms->rooms[i].players[j])==0){
                        users->players[k].inRoom = 0;
                    }
                }
            }
            struct ROOM *tmp = malloc(sizeof(struct ROOM));
            tmp = &rooms->rooms[rooms->roomsCount-1];
            rooms->rooms[i] = *tmp;
            rooms->rooms = realloc(rooms->rooms, (rooms->roomsCount-1)*sizeof(struct ROOM));
            rooms->roomsCount--;
            break;
        }
    }
    pthread_mutex_unlock(&(users->mutex));
    pthread_mutex_unlock(&(rooms->mutex));
    return false;
}

char **returnPlayers(struct USERS *users){
    char **returnTab = malloc(sizeof(char)*MAX_NICK_LENGTH*MAX_USR_COUNT);
    for(int i = 0; i< users->registeredUsersCount; i++){
        strncpy(returnTab[i], users->players[i].nick, MAX_NICK_LENGTH);
    }
    return returnTab;
}

char **returnRooms(struct ROOMS *rooms){
    char **returnTab = malloc(sizeof(char)*MAX_ROOM_NAME*MAX_ROOMS_COUNT);
    for(int i =0; i< rooms->roomsCount;i++){
        strncpy(returnTab[i], rooms->rooms[i].name, MAX_ROOM_NAME);
    }
    return returnTab;
}

char **returnPlayersInRoom(struct ROOMS *rooms, char *roomName){
    char **returnTab = malloc(sizeof(char)*MAX_PLAYERS*MAX_NICK_LENGTH);
    for(int i = 0; i < rooms->roomsCount; i++){
        if(strcmp(rooms->rooms[i].name, roomName) == 0){
            for(int j = 0; j < rooms->rooms[i].playersIn; j++){
                strncpy(returnTab[j], rooms->rooms[i].players[j], MAX_NICK_LENGTH);
            }
            break;
        }
    }
    return returnTab;
}

int pointsForLetter(char *SecretWord, char *InsertedLetters, char letter){
    int len = sizeof(&SecretWord)/sizeof(SecretWord[0]);
    int len_letters = sizeof(&InsertedLetters)/sizeof(InsertedLetters[0]);    
    //Couldn't even happen, players should pick only not used letters
    for(int i = 0; i < len_letters; i++)
        if(InsertedLetters[i]==letter)
            return 0;
    //add new letter
    InsertedLetters = realloc(InsertedLetters, (len_letters+1) * sizeof(char));
    InsertedLetters[len_letters] = letter;
    //check if SecredWord has that letter/letters
    for(int i = 0; i < len; i++)
        if(SecretWord[i]==letter)
            return 1;
    
    return -1;
}

char *returnSecretWord(char *SecretWord, char *InsertedLetters){
    int len = sizeof(&SecretWord)/sizeof(SecretWord[0]);
    int len_letters = sizeof(&InsertedLetters)/sizeof(InsertedLetters[0]);
    char *result = malloc(sizeof(char)*len);
    for (int i = 0; i<len;i++){
        if(SecretWord[i] == ' '){
            result[i] = '-';
        }
        else
            for(int j = 0; j < len_letters; j++){
                if(InsertedLetters[j] == SecretWord[i]){
                    result[i] = InsertedLetters[j];
                    break;
                }
                if(j == len_letters-1)
                    result[i] = '_';
            }
    }
    return result;
}

int stringToInt(char *pin){
    int result = 0;
    
    return result;
}