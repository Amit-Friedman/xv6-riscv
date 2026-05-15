#include "kernel/types.h"
#include "user/user.h"

int main(void){
    lcg_srand(15);
    for(int i = 0;i<7;i++){
        printf("rand: %u\n", lcg_rand());
    }
    return 0;
}