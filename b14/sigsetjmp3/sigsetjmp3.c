#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

static void ssu_signal_handler1(int signo);
static void ssu_signal_handler2(int signo);

sigjmp_buf jmp_buf1;
sigjmp_buf jmp_buf2;

int main(void)
{
	struct sigaction act_sig1;
	struct sigaction act_sig2;
	int i, ret;
	printf("My PID is %d\n", getpid());
	ret = sigsetjmp(jmp_buf1, 1);

	if (ret == 0)
	{
		act_sig1.sa_handler = ssu_signal_handler1;
		sigaction(SIGINT, &act_sig1, NULL);//SIGINT의 handler를 설정해준다.
	}
	else if (ret == 3)//SIGINT의 handelr함수에 의해 ret이 3되면서 setjmp로 점프한다.
		printf("---------------\n");

	printf("Starting\n");
	sigsetjmp(jmp_buf2, 2);//SIGUSR1의 handelr함수에 의해 setjmp로 점프한다.
	act_sig2.sa_handler = ssu_signal_handler2;
	sigaction(SIGUSR1, &act_sig2, NULL);//SIGUSR1의 handler를 설정해준다.

	for (i = 0; i < 20; i++)
	{
		printf("i = %d\n", i);
		sleep(1);
	}

	exit(0);

}

static void ssu_signal_handler1(int signo)
{
	fprintf(stderr, "\nInterrupted\n");
	siglongjmp(jmp_buf1, 3);
}

static void ssu_signal_handler2(int signo)
{
	fprintf(stderr, "\nSIGUSR1\n");
	siglongjmp(jmp_buf2, 2);
}
