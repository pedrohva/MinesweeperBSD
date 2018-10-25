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

struct game* head_gameinfo = NULL;   // HEAD of the linked list of the queue of clients
struct game* tail_gameinfo = NULL;   // TAIL of the linked list of the queue of clients

int username_exists(char* username) {
    // Iterate through the list while checking if the usernames match the one that was passed
    struct user* userinfo = head_userinfo;
    while(userinfo != NULL) {
        if(strcmp(username, userinfo->username) == 0) {
            return 1;
        }
        userinfo = userinfo->next;
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
        gameinfo_size++;
    } else {
        // Check if the score is should be at the top of the list
        if(gameinfo->time_taken > head_gameinfo->time_taken) {
            gameinfo->next = head_gameinfo;
            head_gameinfo = gameinfo;
            gameinfo_size++;
            return;
        }

        // The data for the user who just won a game
        int user_games_won, user_games_played;
        // The data for the current score in the list
        int iterator_games_won, iterator_games_played;

        // Check if the score at the top of the list is equal to the user's score
        if(gameinfo->time_taken == head_gameinfo->time_taken) {
            get_userinfo(gameinfo->username, &user_games_played, &user_games_won);
            get_userinfo(head_gameinfo->username, &iterator_games_played, &iterator_games_won);
            // Place the user at the top of the list if they've won less games
            if(user_games_won < iterator_games_won) {
                gameinfo->next = head_gameinfo;
                head_gameinfo = gameinfo;
                gameinfo_size++;
                return;
            }
        }

        // Iterate through the list to find a spot to place the score
        struct game* game_iterator = head_gameinfo;
        while(game_iterator != NULL) {
            // Check if the score fits after the current item in the list
            if(gameinfo->time_taken <= game_iterator->time_taken) {
                // Check if the scores are equal (to apply the games won rule) AND we can place the score after the current one
                if(gameinfo->time_taken == game_iterator->time_taken) {
                    get_userinfo(gameinfo->username, &user_games_played, &user_games_won);
                    get_userinfo(game_iterator->username, &iterator_games_played, &iterator_games_won);

                    // Insert after the current item in the list if this user has won more games
                    if(user_games_won >= iterator_games_won) {
                        gameinfo->next = game_iterator->next;
                        game_iterator->next = gameinfo;
                        gameinfo_size++;
                        return;
                    }
                }
                // Check if we have reached the tail of the list
                if(game_iterator->next == NULL) {
                    gameinfo->next = NULL;
                    game_iterator->next = gameinfo;
                    gameinfo_size++;
                    return;
                }
                // Check if the score fits between two scores in the middle of the list
                if(gameinfo->time_taken > game_iterator->next->time_taken) {
                    gameinfo->next = game_iterator->next;
                    game_iterator->next = gameinfo;
                    gameinfo_size++;
                    return;
                }

                // Check if the next score will be equal AND we need to place the score before the next one (due to games won rule)
                if(gameinfo->time_taken == game_iterator->next->time_taken) {
                    get_userinfo(gameinfo->username, &user_games_played, &user_games_won);
                    get_userinfo(game_iterator->next->username, &iterator_games_played, &iterator_games_won);

                    if(user_games_won < iterator_games_won) {
                        gameinfo->next = game_iterator->next;
                        game_iterator->next = gameinfo;
                        gameinfo_size++;
                        return;
                    }
                }
            }

            game_iterator = game_iterator->next;
        }
    }
}

void leaderboard_update_user_games(char* username, int game_won) {
    // If the user exists, increment their details accordingly 
    if(username_exists(username)) {
        struct user* userinfo = head_userinfo;
        while(userinfo != NULL) {
            if(strcmp(username, userinfo->username) == 0) {
                userinfo->games_played++;
                if(game_won) {
                    userinfo->games_won++;
                }
                return;
            }
            userinfo = userinfo->next;
        }
    } else { 
        // If the user doesn't exist in the list yet, add to the list
        leaderboard_add_user(username, game_won);
    }
}

void get_userinfo(char* username, int* games_played, int* games_won) {
    *games_played = -1;
    *games_won = -1;

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