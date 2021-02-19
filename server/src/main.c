#include "main.h"

void *worker(void *input)
{
    pthread_detach(pthread_self());
    char nick[MAX_NICK_LENGTH] = {0};
    char roomName[MAX_ROOM_NAME] = {0};
    char request[REQUEST_MAX_LEN] ={0};
    struct THREAD_DATA *thread_data = ((struct THREAD_DATA *)input);
    int connectionSocketDescriptor = thread_data->fd;
    bool running = true;
    if(!loginToServer(connectionSocketDescriptor, thread_data->users, nick))
        return NULL;

    while(running){
        memset(request, 0, REQUEST_MAX_LEN);
        handle_error(read(connectionSocketDescriptor, request, REQUEST_MAX_LEN));
        printf("%s", request);
        if(strcmp(request, GET_ROOM_LIST) == 0){
            sendRoomsList(connectionSocketDescriptor, thread_data->rooms);
        }else if(strcmp(request, NEW_ROOM_REQUEST) == 0){
            lobbyCreateRoom(connectionSocketDescriptor, thread_data->users, thread_data->rooms, nick);
        }else if(strcmp(request, JOIN_ROOM) == 0){
            lobbyJoinRoom(connectionSocketDescriptor, thread_data->users, thread_data->rooms, nick);
        }else if(strcmp(request, LOGOUT) == 0){
            if(leaveServer(connectionSocketDescriptor, thread_data->users, thread_data->rooms, nick));
                return NULL;
        }else if(strcmp(request, PLAYERS_IN_ROOM) == 0){
            sendPlayersList(connectionSocketDescriptor, thread_data->rooms, roomName);
        }else if(strcmp(request, START_GAME) == 0){

        }
    }
    return NULL;
}

bool loginToServer(int connfd, struct USERS *users, char *nick){
    bool status = false;
    int serverStatus=0;
    char request[REQUEST_MAX_LEN]={0};
    
    do{
        writeToClient(connfd, CONNECTION_ESTABLISHED);
        memset(request, 0, REQUEST_MAX_LEN);
        handle_error(read(connfd, request, REQUEST_MAX_LEN));
        if(strcmp(request, LOGIN_REQUEST) == 0){
            writeToClient(connfd, GET_NICK);
            memset(nick, 0, MAX_NICK_LENGTH);
            handle_error(read(connfd, nick, MAX_NICK_LENGTH));

            serverStatus = login(users, nick);
            switch (serverStatus)
            {
            case 0:
                writeToClient(connfd, LOGIN_SUCCESSFULL);
                return true;
            case 1:
                writeToClient(connfd, LOGIN_CHANGE_NAME);
                return false;
            case 2:
                writeToClient(connfd, LOGIN_USER_LIMITS);
                return false;
            }
        }
    }while(!status);
    return false;
}

bool leaveServer(int connfd, struct USERS* users, struct ROOMS *rooms, char *nick){
    bool status = false;
    int serverStatus=0;
    char nick2[MAX_NICK_LENGTH]={0};
    writeToClient(connfd, GET_NICK);
    handle_error(read(connfd, nick2, MAX_NICK_LENGTH));
    if(strcmp(nick2, nick)==0){
        serverStatus = logout(users, nick);
        printf("%d", serverStatus);
        switch (serverStatus)
        {
        case 0:
            writeToClient(connfd, LOGOUT_SUCCESSFULL);
            return true;
        case 1:
            writeToClient(connfd, PLAYER_IN_GAME);
            return false;
        case 2:
            writeToClient(connfd, LOGOUT_FAILED);  
            return false;
        }
    }
    return false;
}

bool lobbyCreateRoom(int connfd, struct USERS *users, struct ROOMS *rooms, char *nick){
    bool status = false;
    char secredWord[MAX_SECRET_WORD]={0};
    char pin[PIN]={0};
    int serverStatus = 0;
    char request[REQUEST_MAX_LEN];

    char roomName[MAX_ROOM_NAME]={0}; 

        memset(request, 0, REQUEST_MAX_LEN);
        writeToClient(connfd, GET_ROOM_NAME);
        memset(roomName, 0, MAX_ROOM_NAME);
        handle_error(read(connfd, roomName, MAX_ROOM_NAME));

        writeToClient(connfd, GET_SECRED_WORD);
        memset(secredWord, 0, MAX_SECRET_WORD);
        handle_error(read(connfd, secredWord, MAX_SECRET_WORD));

        writeToClient(connfd, GET_PIN);
        memset(pin, 0, PIN);
        handle_error(read(connfd, pin, PIN));

        serverStatus = createRoom(rooms, users, roomName, nick, secredWord, atoi(pin));
        switch (serverStatus)
        {
        case 0:
            writeToClient(connfd, ROOM_CREATED);
            return true;
        case 1:
            writeToClient(connfd, ROOM_CHANGE_NAME);
            return false;
            break;
        case 2:
            writeToClient(connfd, ROOM_LIMITS);
            return false;
        case 3:
            writeToClient(connfd, ROOM_CREATING_FAILED);
            return false;
        }
        
    return false;
}

bool lobbyJoinRoom(int connfd, struct USERS *users, struct ROOMS *rooms, char *nick){
    bool statlogous = false;
    char pin[PIN]={0};
    int serverStatus = 0;
    char request[REQUEST_MAX_LEN];
    char joinRoomName[MAX_ROOM_NAME]={0};

    memset(request, 0, REQUEST_MAX_LEN);
    writeToClient(connfd, GET_ROOM_NAME);
    memset(joinRoomName, 0, MAX_ROOM_NAME);
    handle_error(read(connfd, joinRoomName, MAX_ROOM_NAME));

    writeToClient(connfd, GET_PIN);
    memset(pin, 0, PIN);
    handle_error(read(connfd, pin, PIN));

    serverStatus = joinRoom(rooms, users, nick, joinRoomName, atoi(pin));
    switch (serverStatus)
        {
        case 0:
            writeToClient(connfd, JOINED_ROOM);
            return true;
        case 1:
            writeToClient(connfd, ROOM_IS_FULL);
            return false;
        case 2:
            writeToClient(connfd, WRONG_PIN);
            return false;
        case 3:
            writeToClient(connfd, ROOM_NOT_EXISTS);
            return false;
        }
}

void sendRoomsList(int connfd, struct ROOMS *rooms){
    char request[REQUEST_MAX_LEN]={0};
    char count[5]={0};
        sprintf(count, "%d", rooms->roomsCount);
        writeToClient(connfd, count);
        writeToClient(connfd, "\n");
        for(int i = 0; i < rooms->roomsCount; i++){
            char name[MAX_ROOM_NAME]={0};
            strncpy(name, rooms->rooms[i].name, MAX_ROOM_NAME);
            replaceDelimiter(name);
            writeToClient(connfd, name);

            char playersInRoom[2]={0};
            sprintf(playersInRoom, "%d", rooms->rooms[i].playersIn);
            writeToClient(connfd, playersInRoom);
            writeToClient(connfd, "\t");
            }
        writeToClient(connfd, "\n");
}

void sendPlayersList(int connfd, struct ROOMS *rooms, char *roomName){
    char request[REQUEST_MAX_LEN]={0};
    char count[2]={0};
    handle_error(read(connfd, request, REQUEST_MAX_LEN));
    if(strcmp(request, GET_PLAYER_LIST) == 0)
        for(int i = 0; i < rooms->roomsCount; i++){
            if(strcmp(rooms->rooms[i].name, roomName) == 0){
                sprintf(count, "%d", rooms->rooms[i].playersIn);
                writeToClient(connfd, count);
                writeToClient(connfd, "\n");
                for(int j = 0; j < rooms->rooms[i].playersIn; j++){
                    char name[MAX_NICK_LENGTH]={0};
                    strncpy(name, rooms->rooms[i].players[j], MAX_NICK_LENGTH);
                    replaceDelimiter(name);
                    writeToClient(connfd, name);
                    writeToClient(connfd, "\t");
                }
                return;
            }    
        }
}

void newUser(int connfd, struct USERS *users, struct ROOMS *rooms)
{
    pthread_t thread;
    printf("Nowy %d\n", connfd);
    struct THREAD_DATA *thread_data = malloc(sizeof(struct THREAD_DATA));
    thread_data->fd = connfd;
    thread_data->rooms = rooms;
    thread_data->users = users;
    handle_error(pthread_create(&thread, NULL, worker, (void *)thread_data));
}

int main(int argc, char *argv[])
{
    int sockfd, connfd, len, on = 1;
    struct sockaddr_in servaddr;
    struct USERS *users = malloc(sizeof(struct USERS));
    struct ROOMS *rooms = malloc(sizeof(struct ROOMS));
    initRooms(rooms);
    initUsers(users);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    servaddr.sin_port = htons(SERVER_PORT);

    // bind to socket
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    // start listening
    if ((listen(sockfd, QUEUE_SIZE)) != 0)
    {
        printf("Listening failed...\n");
        exit(0);
    }

    printf("Starting server at %s:%d\n", SERVER_ADDR, SERVER_PORT);
    while (1)
    {
        connfd = accept(sockfd, NULL, NULL);
        handle_error(connfd);
        newUser(connfd, users, rooms);
    }
    close(sockfd);
    free(users);
    free(rooms);
    return 0;
}