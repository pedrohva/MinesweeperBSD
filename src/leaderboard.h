#ifndef LEADERBOARD_H
#define LEADERBOARD_H

/**
 * This list contains information about each unique user that plays Minesweeper. The unique 
 * identifier for each user is their username.
 * The information includes:
 *      - Games played
 *      - Games won
 **/
struct user {
    char* username;         
    int games_won;
    int games_played;     
    struct user* next; // Pointer to the next user
};

/**
 * Contains a list of won games with the time taken to complete. Contains the username of 
 * whoever won each specific game to allow joining of the two lists. 
 **/
struct game {
    char* username; 
    int time_taken;       
    struct game* next;   // Pointer to the next game completed
};

void leaderboard_free();

void leaderboard_add_score(char* username, int time_taken);

void leaderboard_update_user_games(char* username, int game_won);

struct game* get_gameinfo_head();

void get_userinfo(char* username, int* games_played, int* games_won);

int get_userinfolist_size();

int get_gameinfo_size();

#endif // LEADERBOARD_H