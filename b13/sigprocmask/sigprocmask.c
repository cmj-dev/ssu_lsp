#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
int main(void)
{
    sigset_t sig_set;
    int count;

    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGINT);//비어있는 sigset 집합에 sigint를 추가한다.
    sigprocmask(SIG_BLOCK, &sig_set, NULL);//sigset안에 포함되어 있는 원소들을 block하게 한다.
//그러므로 sigset에 있는 sigint signal이 block될 것이다.
    for (count = 3; 0 < count; count--)
    {
        printf("count %d\n", count);
        sleep(1);
    }

    printf("Ctrl-C에 대한 블록을 해제\n");
    sigprocmask(SIG_UNBLOCK, &sig_set, NULL);//sig unblock을 이용해서 sig set에 있는 sigint에 대한 block을 해제한다.
    printf("count중 Ctrl-C입력하면 이 문장은 출력 되지 않음.\n");//block되어있던 sigint가 존재하고 있으면 sigint가 곧바로 적용되어 출력되지 않는다.

    while (1);

    exit(0);
}