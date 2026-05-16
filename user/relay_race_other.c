#include "kernel/types.h"
#include "user/user.h"

void gameloop(int gid, int lockid)
{
    int team_id = gid;
    int pid = getpid();
    while (!is_game_over())
    {
        israeli_acquire(lockid);
        if (is_game_over())
        {
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
    int c = 50;
    int lock_id = israeli_create(c);
    init_scores();
    // 3 teams of 5
    printf("-----------------FAVORITISM: %d-----------------\n", c);
    printf("------------5 teams of 3 players---------------\n");
    if (fork() == 0)
    { // childA
        setgid(0);
        if (fork() == 0)
        { // create 3 for each gid;
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
            { // create 3 for each gid;
                fork();
            }
            gameloop(1, lock_id);
        }
        else
        {
            if (fork() == 0)
            { // childC
                setgid(2);
                if (fork() == 0)
                { // create 3 for each gid;
                    fork();
                }
                gameloop(2, lock_id);
            }
            else
            {
                if (fork() == 0)
                { // childD
                    setgid(3);
                    if (fork() == 0)
                    { // create 3 for each gid;
                        fork();
                    }
                    gameloop(3, lock_id);
                }

                else
                { // parent
                    setgid(4);
                    if (fork() == 0)
                    { // create 3 for each gid;
                        fork();
                    }
                    gameloop(4, lock_id);
                }
            }
        }
    }
    return 0;
}