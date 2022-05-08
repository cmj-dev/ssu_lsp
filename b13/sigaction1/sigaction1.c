#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
void ssu_signal_handler(int signo)
{
    printf("ssu_signal_handler control\n");
}

int main(void)
{
    struct sigaction sig_act;
    sigset_t sig_set;

    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;
    sig_act.sa_handler = ssu_signal_handler;//sig_act라는 구조체에 handler를 넣어준다.
    sigaction(SIGUSR1, &sig_act, NULL);//SIGUSR1 signal이 도착하면 sigact안에 있는 handler가 실행되게 된다.
    printf("before first kill()\n");
    kill(getpid(), SIGUSR1);//해당 프로세스에 대해 sigusr1 시그널을 보낸다.
    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &sig_set, NULL);//SIGUSR1을 block처리하기 때문에 kill sigusr1을 하더라도 handler가 실행되지 않는다.
    printf("before second kill()\n");
    kill(getpid(), SIGUSR1);//해당 프로세스에 대해 sigusr1 시그널을 보낸다.
    printf("after second kill()\n");
    exit(0);
}