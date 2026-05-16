#include "kernel/types.h"
#include "user/user.h"

void gameloop(int gid, int lockid)
{
    int team_id = gid;
    int pid = getpid();
    while (!is_game_over())
    {
        israeli_acquire(lockid);
        if(is_game_over()){
            israeli_release(lockid);
            return;
        } 
        printf("[process id]:%d     [team id]: %d       [score]: %d \n\n", pid, team_id, increment_score(team_id));
        israeli_release(lockid);
        sleep(1);
    }
}

int main(void)
{
    int c = 100;
    int lock_id = israeli_create(c);
    init_scores();
    // 3 teams of 5
    printf("-----------------FAVORITISM: %d-----------------\n", c);
    printf("------------3 teams of 5 players---------------\n");
    if (fork() == 0)
    { // childA
        setgid(0);
        if (fork() == 0)
        { // create 5 for each gid;
            fork();
            fork();
        }
        gameloop(0, lock_id);
    }
    else
    {
        if (fork() == 0)
        { // childB
            setgid(1);
            if (fork() == 0)
            { // create 5 for each gid;
                fork();
                fork();
            }
            gameloop(1, lock_id);
        }
        else
        { // parent
            setgid(2);
            if (fork() == 0)
            { // create 5 for each gid;
                fork();
                fork();
            }
            gameloop(2, lock_id);
        }
    }
    return 0;
}