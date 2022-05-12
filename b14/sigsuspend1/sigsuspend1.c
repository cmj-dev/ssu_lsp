#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t old_set;
	sigset_t sig_set;

	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGINT);
	sigprocmask(SIG_BLOCK, &sig_set, &old_set);//SIGINT를 BLOCK설정한다
	sigsuspend(&old_set);//old_set집합으로 SIG_BLOCK을 하기 때문에 SIGINT가 UNBLOCK되고 SIGINT의 핸들러가 실행 가능해진다.
	exit(0);
}
