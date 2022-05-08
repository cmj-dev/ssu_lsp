#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
int main(void)
{
    sigset_t pendingset;
    sigset_t sig_set;
    int count = 0;
    sigfillset(&sig_set);
    /* 위의 코드와 같이 
    sigfillset(&sig_set);
    보다는
    sigemptyset(&sig_set)
    sigaddset(&sig_set, SIGINT);
    위와 같은 코드가 좀 더 명확하다.
    */
    sigprocmask(SIG_SETMASK, &sig_set, NULL);
    while (1)
    {
        printf("count: %d\n", count++);
        sleep(1);
        if (sigpending(&pendingset) == 0)//현재 블록되어 대기중인 signal이 pendingset에 저장된다.
        {
            if (sigismember(&pendingset, SIGINT))//블록되어 signal이 도착한 후에 대기중인 시그널이 sigint인지 확인한다.
            {
                printf("SIGINT가 블록되어 대기 중. 무한 루프를 종료.\n");
                break;
            }
        }
    }
    exit(0);
}