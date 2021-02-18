#include "main.h"

void *worker(void *input)
{
    pthread_detach(pthread_self());
    char nick[MAX_NICK_LENGTH] = {0};
    char roomName[MAX_ROOM_NAME] = {0};
    struct THREAD_DATA *thread_data = ((struct THREAD_DATA *)input);
    int connectionSocketDescriptor = thread_data->fd;
    
    bool k = false;
    do{k = loginToServer(connectionSocketDescriptor, thread_data->users, nick);}
    while(!k);
    do{k = lobby(connectionSocketDescriptor, thread_data->users, thread_data->rooms, nick, roomName);}
    while(!k);
    

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
                break;
            case 1:
                writeToClient(connfd, LOGIN_CHANGE_NAME);
                memset(nick, 0, MAX_NICK_LENGTH);
                break;
            case 2:
                writeToClient(connfd, LOGIN_USER_LIMITS);
                return false;
            }
        }
    }while(!status);
    return false;
}

bool lobby(int connfd, struct USERS *users, struct ROOMS *rooms, char *nick, char *roomName){
    bool status = false;
    char secredWord[MAX_SECRET_WORD]={0};
    char pin[PIN]={0};
    int serverStatus = 0;
    char request[REQUEST_MAX_LEN];

    char joinRoomName[MAX_ROOM_NAME]={0};
    do{
        writeToClient(connfd, CREATE_OR_JOIN_ROOM);
        memset(request, 0, REQUEST_MAX_LEN);
        handle_error(read(connfd, request, REQUEST_MAX_LEN));

        if(strcmp(request, NEW_ROOM_REQUEST) == 0){    
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
                break;
            case 2:
                writeToClient(connfd, ROOM_LIMITS);
                return false;
            case 3:
                writeToClient(connfd, ROOM_CREATING_FAILED);
                return false;
            }
        }
        else if(strcmp(request, JOIN_ROOM) == 0){
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
                break;
            case 3:
                writeToClient(connfd, ROOM_NOT_EXISTS);
                return false;
            }
            
        }

    }while(!status);
    return false;
}

void newUser(int connfd, struct USERS *users, struct ROOMS *rooms)
{
    int result = 0;
    pthread_t thread;

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