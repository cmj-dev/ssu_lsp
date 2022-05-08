#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void ssu_signal_handler1(int signo);
static void ssu_signal_handler2(int signo);

int main(void)
{
    struct sigaction act_int, act_quit;

    act_int.sa_handler = ssu_signal_handler1;
    sigemptyset(&act_int.sa_mask);
    sigaddset(&act_int.sa_mask, SIGQUIT);
    act_quit.sa_flags = 0;

    if (sigaction(SIGINT, &act_int, NULL) < 0)
    {
        fprintf(stderr, "sigaction(SIGINT) error\n");
        exit(1);
    }
    act_quit.sa_handler = ssu_signal_handler2;
    sigemptyset(&act_quit.sa_mask);
    sigaddset(&act_quit.sa_mask, SIGINT);
    act_int.sa_flags = 0;

    if (sigaction(SIGQUIT, &act_quit, NULL) < 0)
    {
        fprintf(stderr, "sigaction(SIGQUIT) error\n");
        exit(1);
    }
    //각각 sigquit와 sigint에서 해당 시그널을 받으면 실행하는 동안 상대 시그널을 block하고 있는다.
    pause();//signal을 받을때 까지 기다린다.
    exit(0);//sigquit와 sigint 모두 시그널을 받고 3초간 기다릴때 그 사이에 signal이 들어오지 않으면 처리가 끝나고 종료된다.
    //이때 같은 시그널은 중복되어서 처리되지 않기 때문에 sigint가 실행중일때 sigint나 sigquit가 들어오면 3초 후 다시 처리된다.
}

static void ssu_signal_handler1(int signo)
{
    printf("Signal handler of SIGINT : %d\n", signo);
    printf("SIGQUIT signal is blocked : %d\n", signo);
    printf("sleeping 3 sec\n");
    sleep(3);
    printf("Signal handler of SIGINT ended\n");
}

static void ssu_signal_handler2(int signo)
{
    printf("Signal handler of SIGQUIT : %d\n", signo);
    printf("SIGINT signal is blocked : %d\n", signo);
    printf("sleeping 3 sec\n");
    sleep(3);
    printf("Signal handler of SIGQUIT ended\n");
}