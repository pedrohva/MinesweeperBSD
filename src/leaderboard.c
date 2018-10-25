#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leaderboard.h"

// Contains each user's number of games 
int userinfo_size = 0;  // How many clients are waiting to connect
// Contains each won game
int gameinfo_size = 0;  // How many clients are waiting to connect

struct user* head_userinfo = NULL;   // HEAD of the linked list of the user details
struct user* tail_userinfo = NULL;   // TAIL of the linked list of the user details

struct game* head_gameinfo = NULL;  // HEAD of the linked list of the queue of clients
struct game* tail_gameinfo = NULL;  // TAIL of the linked list of the queue of clients

int username_exists(char* username) {
    // Iterate through the list while checking if the usernames match the one that was passed
    if(userinfo_size > 0) {
        struct user* userinfo = head_userinfo;
        // Check if we haven't reached the end of the list
        if(userinfo != NULL) {
            if(strcmp(username, userinfo->username) == 0) {
                return 1;
            }
            userinfo = userinfo->next;
        }
    }

    return 0;
}

void leaderboard_add_user(char* username, int game_won) {
    // Create the game structure
    struct user* userinfo = (struct user*)malloc(sizeof(struct user));
    if(!userinfo) {
        perror("Error adding user to the leaderboard: out of memory");
        exit(1);
    }
    userinfo->username = username;
    userinfo->games_played = 1;
    userinfo->games_won = game_won;

    // Add the user to the user info list
    if(userinfo_size == 0) {
        head_userinfo = userinfo;
        tail_userinfo = userinfo;
    } else {
        tail_userinfo->next = userinfo;
        tail_userinfo = userinfo;
    }
    userinfo_size++;
}

void leaderboard_add_score(char* username, int time_taken) {
    // Create the game structure
    struct game* gameinfo = (struct game*)malloc(sizeof(struct game));
    if(!gameinfo) {
        perror("Error adding score to the leaderboard: out of memory");
        exit(1);
    }
    gameinfo->username = username;
    gameinfo->time_taken = time_taken;

    // Add the score to the game info leaderboard
    if(gameinfo_size == 0) {
        // Just add the score if the list is empty
        head_gameinfo = gameinfo;
        tail_gameinfo = gameinfo;
    } else {
        // Add the score while sorting the list
        tail_gameinfo->next = gameinfo;
        tail_gameinfo = gameinfo;
    }
    gameinfo_size++;
}

void leaderboard_update_user_games(char* username, int game_won) {
    // If the user exists, increment their details accordingly 
    if(username_exists(username)) {
        struct user* userinfo = head_userinfo;
        for(int i=0; i<=userinfo_size; i++) {
            if(userinfo != NULL) {
                if(strcmp(username, userinfo->username) == 0) {
                    userinfo->games_played++;
                    if(game_won) {
                        userinfo->games_won++;
                    }
                    return;
                }
                userinfo = userinfo->next;
            }
        }
    } else { 
        // If the user doesn't exist in the list yet, add to the list
        leaderboard_add_user(username, game_won);
    }
}

void get_userinfo(char* username, int* games_played, int* games_won) {
    *games_played = 0;
    *games_won = 0;

    if(username_exists(username)) {
        struct user* userinfo = head_userinfo;
        while(userinfo != NULL) {
            if(strcmp(username, userinfo->username) == 0) {
                *games_played = userinfo->games_played;
                *games_won = userinfo->games_won;
                return;
            }
            userinfo = userinfo->next;
        }
    }
}

struct game* get_gameinfo_head() {
    return head_gameinfo;
}

int get_userinfolist_size() {
    return userinfo_size;
}

int get_gameinfo_size() {
    return gameinfo_size;
}