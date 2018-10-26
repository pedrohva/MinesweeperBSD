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

/**
 * Fress all of the memory allocated to the nodes in the two lists. Will also set the head and 
 * tail pointers to NULL.
 **/
void leaderboard_free();

/**
 * Add a record of a won game to the leaderboard. This new score is automatically
 * sorted during the insert operation following rules set by the task sheet.
 **/
void leaderboard_add_score(char* username, int time_taken);

/**
 * Update the record of games played and won by the user passed to this function.
 * Will increment the number of games played by 1 and will increment games won depending if the game was won as passed to the function.
 * If the username does not exist in the leaderboard, it will add it and set games played to 1. 
 **/
void leaderboard_update_user_games(char* username, int game_won);

/**
 * Get the number of games played and won by an user in the leaderboard.
 * If the user does not exists, the values for both variables will be -1.
 **/
void get_userinfo(char* username, int* games_played, int* games_won);

/**
 * Get the HEAD pointer for the game info list
 **/
struct game* get_gameinfo_head();

/**
 * Get the number of users in the leaderboard
 **/
int get_userinfolist_size();

/**
 * Get the number of games won in the leaderboard
 **/
int get_gameinfo_size();

#endif // LEADERBOARD_H