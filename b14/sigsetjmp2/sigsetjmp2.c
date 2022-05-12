#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>

static void ssu_alarm(int signo);
static void ssu_func(int signo);
void ssu_mask(const char *str);

static volatile sig_atomic_t can_jump;
static sigjmp_buf jump_buf;

int main(void)
{
	if (signal(SIGUSR1, ssu_func) == SIG_ERR)
	{
		fprintf(stderr, "SIGUSR1 error\n");
		exit(1);
	}

	if (signal(SIGALRM, ssu_alarm) == SIG_ERR)
	{
		fprintf(stderr, "SIGALRM error\n");
		exit(1);
	}

	ssu_mask("starting main: ");//처음에는 아무런 시그널이 없으므로 출력값이 없다.

	if (sigsetjmp(jump_buf, 1))//SIGUSR1을 실행 후 실행된다.
	{
		ssu_mask("ending main: ");//어떠한 signal도 받지 않은 상태이므로 출력값이 없다.
		exit(0);
	}

	can_jump = 1;

	while(1)
		pause();//시그널을 기다린다.

	exit(0);//이 코드는 실행되지 않는다.
}

void ssu_mask(const char *str)
{
	sigset_t sig_set;
	int err_num;

	err_num = errno;

	if (sigprocmask(0, NULL, &sig_set) < 0)//현재 mask된 시그널들을 sigset에 넣는다.
	{
		printf("sigprocmask() error\n");
		exit(1);
	}

	printf("%s",str);

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
	time_t start_time;

	if (can_jump == 0)
		return;

	ssu_mask("starting ssu_func : ");//현재 SIGUSR1이 실행중이므로 mask된 시그널은 	SIGUSR1이다.
	alarm(3);
	start_time = time(NULL);

	while(1)
		if (time(NULL) > start_time + 5)
			break;

	ssu_mask("ending ssu_func : ");//SIGALRM은 종료되었으므로 SIGUSR1만 mask된 상태이다.
	can_jump = 0;
	siglongjmp(jump_buf, 1);//setjmp로 넘어간다.
}

static void ssu_alarm(int signo)
{
	ssu_mask("in ssu_alarm : ");//SIGUSR1에서 SIGALRM을 실행했기 때문에 현재 mask된 시그널은 SIGUSR1, SIGALRM이다.
}
