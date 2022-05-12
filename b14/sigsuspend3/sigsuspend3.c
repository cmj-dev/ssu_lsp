#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void ssu_signal_handler(int signo);
void ssu_timestamp(char *str);

int main(void)
{
	struct sigaction sig_act;
	sigset_t blk_set;

	sigfillset(&blk_set);
	sigdelset(&blk_set, SIGALRM);
	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_flags = 0;
	sig_act.sa_handler = ssu_signal_handler;
	sigaction(SIGALRM, &sig_act, NULL);//SIGALRM이 오면 sig_signal_handler를 실행한다.
	ssu_timestamp("before sigsuspend()");
	alarm(5);//5초 후 SIGALRM SIGNAL이 발생한다.
	sigsuspend(&blk_set);//SIGALRM이 발생하면 해당 SIGNAL을 처리하고 SUSPEND가 종료된다.
	ssu_timestamp("after sigsuspend()");
	exit(0);
}

void ssu_signal_handler(int signo)
{
	printf("in ssu_signal_handler() function\n");
}

void ssu_timestamp(char *str)
{
	time_t time_val;

	time(&time_val);
	printf("%s the time is %s\n", str, ctime(&time_val));
}
