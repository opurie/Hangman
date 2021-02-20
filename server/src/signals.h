#ifndef SIGNALS_H
#define SIGNALS_H

#define REQUEST_MAX_LEN 255
//server responses
#define CONNECTION_ESTABLISHED "Connected\n"
#define GET_NICK "Get nick\n"
#define LOGIN_SUCCESSFULL "Logged in\n"
#define LOGIN_CHANGE_NAME "Login failed, change name\n"
#define LOGIN_USER_LIMITS "Login failed, users limits\n"
#define LOGOUT_SUCCESSFULL "Logout successfull\n"
#define PLAYER_IN_GAME "Player in game\n"
#define LOGOUT_FAILED "Logout failed\n"

#define CREATE_OR_JOIN_ROOM "Create or join room\n"
#define GET_ROOM_NAME "Get room name\n"
#define GET_PIN "Get pin\n"
#define GET_SECRED_WORD "Get secred word\n"
#define ROOM_CREATED "Room created\n"
#define ROOM_CHANGE_NAME "Change room name\n"
#define ROOM_LIMITS "Room limits\n"
#define ROOM_CREATING_FAILED "Room creating failed\n"
#define JOIN_ROOM "Join room\n"
#define JOINED_ROOM "Joined room\n"
#define WRONG_PIN "Wrong pin\n"
#define ROOM_IS_FULL "Room is full\n"
#define ROOM_NOT_EXISTS "Room not exists\n"
#define ROOM_LEAVED "Room leaved\n"
#define ROOM_NOT_LEAVED "Room not leaved\n"

#define USER_NOT_FOUND "User not found\n"
#define ROOM_NOT_FOUND "Room not found\n"

#define GAME_STARTING "Game starting\n"
#define TIMER "Timer\n"
#define WINNER_INFO "The winner is\n"
#define WHO_GOT_POINT "Point to\n"
#define ROOMS_LIST "Rooms list\n"
#define USERS_LIST "Users list\n"
#define PLAYERS_IN_ROOM "Players in room\n"



//client requests
#define LOGIN_REQUEST "Register\n"
#define NEW_ROOM_REQUEST "New Room\n"
#define START_GAME "Start game\n"
#define LOGOUT "Logout\n"
#define JOIN_ROOM "Join room\n"
#define LEAVE_ROOM "Leave room\n"
#define DELETE_ROOM "Delete room\n"

#define GET_ROOM_LIST "Get room list\n"
#define GET_PLAYER_LIST "Get player list\n"

#define PICK_LETTER "Pick letter\n" // pick letter A-Z

#endif