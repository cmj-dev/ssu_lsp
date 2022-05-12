#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

static void ssu_func(int signo);
void ssu_print_mask(const char *str);

int main(void)
{
	sigset_t new_mask, old_mask, wait_mask;
	
	ssu_print_mask("program start : ");

	if (signal(SIGINT, ssu_func) == SIG_ERR)
	{
		fprintf(stderr, "signal(SIGINT) error\n");
		exit(1);
	}
	
	sigemptyset(&wait_mask);
	sigaddset(&wait_mask, SIGUSR1);
	sigemptyset(&new_mask);
	sigaddset(&new_mask, SIGINT);

	if (sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0)//SIGINT를 BLOCK한다.
	{
		fprintf(stderr, "SIG_BLOCK() error\n");
		exit(1);
	}

	ssu_print_mask("in critical region : ");//현재 BLOCK된 SIGNAL이 SIGINT만 있기 때문에 SIGINT가 출력된다.

	if (sigsuspend(&wait_mask) != -1)//SIGUSR를 BLOCK시키고 SIGINT가 오거나 읽을 수 있는 signal이 올때까지 pause한다.
	{
		fprintf(stderr, "sigsuspend() error\n");
		exit(1);
	}

	ssu_print_mask("after return from sigsuspend : ");//다시 그 전 상태로 돌아가기 때문에 in critical region과 같은 값이 출력된다.

	if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0)
	{
		fprintf(stderr, "SIG_SETMASK() error\n");
		exit(1);
	}

	ssu_print_mask("program exit : ");//sigprocmask에 의해 block된 signal이 없어진다.
	exit(0);
}

void ssu_print_mask(const char *str)
{
	sigset_t sig_set;
	int err_num;

	err_num = errno;

	if (sigprocmask(0, NULL, &sig_set) < 0)
	{
		fprintf(stderr, "sigprocmask() error\n");
		exit(1);
	}

	printf("%s", str);

	if (sigismember(&sig_set, SIGINT))
		printf("SIGINT ");

	if (sigismember(&sig_set, SIGQUIT))
		printf("SIGQUIT ");
	
	if (sigismember(&sig_set, SIGUSR1))
		printf("SIGUSR1 ");
	
	if (sigismember(&sig_set, SIGALRM))
		printf("SIGALRM ");

	printf("\n");
	errno = err_num;
}

static void ssu_func(int signo)
{
	ssu_print_mask("\nin ssu_func : ");//sigsuspend중 sigint를 실행하면 sigint와 sigusr1이 block된 상태이다.
}
