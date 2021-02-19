/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package wisielecclient;


public class Requests {
    public int REQUEST_MAX_LEN = 255;
//server responses
    public String CONNECTION_ESTABLISHED  ="Connected";
    public String GET_NICK = "Get nick";
    public String LOGIN_SUCCESSFULL = "Logged in";
    public String LOGIN_CHANGE_NAME = "Login failed, change name";
    public String LOGIN_USER_LIMITS = "Login failed, users limits";
    public String LOGOUT_SUCCESSFULL = "Logout successfull";
    public String PLAYER_IN_GAME = "Player in game";
    public String LOGOUT_FAILED = "Logout failed";
    
    public String CREATE_OR_JOIN_ROOM = "Create or join room";
    public String GET_ROOM_NAME = "Get room name";
    public String GET_PIN = "Get pin";
    public String GET_SECRED_WORD = "Get secred word";
    public String ROOM_CREATED = "Room created";
    public String ROOM_CHANGE_NAME = "Change room name";
    public String ROOM_LIMITS = "Room limits";
    public String ROOM_CREATING_FAILED = "Room creating failed";
    public String JOIN_ROOM = "Join room";
    public String JOINED_ROOM = "Joined room";
    public String WRONG_PIN = "Wrong pin";
    public String ROOM_IS_FULL  ="Room is full";
    public String ROOM_NOT_EXISTS = "Room not exists";

    public String USER_NOT_FOUND = "User not found";
    public String ROOM_NOT_FOUND = "Room not found";

    public String TIMER = "Timer";
    public String WINNER_INFO = "The winner is";
    public String WHO_GOT_POINT = "Point to";
    public String ROOMS_LIST = "Rooms list";
    public String USERS_LIST = "Users list";
    public String PLAYERS_IN_ROOM = "Players in room";



    //client requests
    public String LOGIN_REQUEST = "Register";
    public String NEW_ROOM_REQUEST = "New Room";
    public String START_GAME = "Start game";
    public String LOGOUT = "Logout";
    public String LEAVE_ROOM = "Leave room";
    public String DELETE_ROOM = "Delete room";

    public String GET_ROOM_LIST = "Get room list";
    public String GET_PLAYER_LIST ="Get player list";

    public String PICK_LETTER ="Pick letter"; // pick letter A-Z
    
    public Requests(){}


}
