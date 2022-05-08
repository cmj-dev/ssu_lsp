#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_check_pending(int signo, char *signame);
void ssu_signal_handler(int signo);

int main(void)
{
    struct sigaction sig_act;
    sigset_t sig_set;

    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;
    sig_act.sa_handler = ssu_signal_handler;

    if (sigaction(SIGUSR1, &sig_act, NULL) != 0)//error 처리를 해주면서 sigusr1에 ssu_signal_handler를 핸들러로 지정해준다.
    {
        fprintf(stderr, "sigaction() error\n");
        exit(1);
    }

    else
    {
        sigemptyset(&sig_set);
        sigaddset(&sig_set, SIGUSR1);

        if (sigprocmask(SIG_SETMASK, &sig_set, NULL) != 0)//error처리를 해주면서 sig_set 집합 안에 있는 sigusr1을 block처리한다.
        {
            fprintf(stderr, "sigprocmask() error\n");
            exit(1);
        }
        else
        {
            printf("SIGUSR1 signals are now blocked\n");
            kill(getpid(), SIGUSR1);
            printf("after kill()\n");
            ssu_check_pending(SIGUSR1, "SIGUSR1");
            sigemptyset(&sig_set);
            sigprocmask(SIG_SETMASK, &sig_set, NULL);//procmask로 emptyset을 설정해주므로 모든 signal이 block되지 않는 상태이다.
            printf("SIGUSR1 signals are no longer blocked\n");
            ssu_check_pending(SIGUSR1, "SIGUSR1");
        }
    }
    exit(0);
}

void ssu_check_pending(int signo, char *signame)
{
    sigset_t sig_set;

    if (sigpending(&sig_set) != 0)//현재 프로세스에서 block되어 대기중인 집합들을 sig_set에 넣어놓는다.
        printf("sigpending() error\n");
    else if (sigismember(&sig_set, signo))//해당 signo가 sig_set에 있으면 현재 블록되어 대기중인 signal이므로 pending되었다고 확인해준다.
        printf("a %s signal is pending\n", signame);
    else
        printf("%s signals are not pending\n", signame);
}

void ssu_signal_handler(int signo)
{
    printf("in ssu_signal_handler function\n");
}