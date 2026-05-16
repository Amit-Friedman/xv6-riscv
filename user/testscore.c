#include "kernel/types.h"
#include "user/user.h"

int main(void){
    init_scores();
    printf("%d", get_score(0));
    printf("\n");
    printf("%d", increment_score(0));
    printf("\n");
    printf("%d", increment_score(1));
    printf("\n");
    printf("%d", increment_score(0));
    printf("\n");
    printf("%d", get_score(0));
    printf("\n");
    return 0;
}