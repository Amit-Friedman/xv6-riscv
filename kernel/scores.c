#define MAX_TEAMS 5
#define TARGET_SCORE 30

int scores[MAX_TEAMS];
int game_over;

void
init_scores(void){
    for(int i = 0;i<MAX_TEAMS;i++){
        scores[i] = 0;
    }
    game_over = 0;
}

int
get_score(int team_id){
    return scores[team_id];
}


int
increment_score(int team_id){
    scores[team_id]++;
    if(scores[team_id] == TARGET_SCORE) game_over = 1;
    return scores[team_id];
}

int 
is_game_over(void){
    return game_over;
}
