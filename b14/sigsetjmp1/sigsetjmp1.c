#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

void ssu_signal_handler(int signo);

jmp_buf jump_buffer;

int main(void)
{
	signal(SIGINT, ssu_signal_handler);

	while(1)
	{
		if (setjmp(jump_buffer) == 0)
		{
			printf("Hit Ctrl-C at anytime ..\n");
			pause();//longjmp에 의해 setjmp로 돌아오면 signal_handler가 닫히지 않았기 때문에 Ctrl-C가 계속 block된다.
		}
	}

	exit(0);
}

void ssu_signal_handler(int signo)
{
	char character;

	signal(signo, SIG_IGN);//해당 시그널 핸들러가 호출되면 호출시킨 시그널을 블럭시킨다
	printf("Did you hit Ctrl-C?\n" "Do you really want to quit? [y/n] ");
	character = getchar();

	if (character == 'y' || character == 'Y')
		exit(0);//사용자가 실제로 Ctrl-C를 보냈으면 정상적으로 종료한다.
	else
	{
		signal(SIGINT, ssu_signal_handler);//Ctrl-C를 사용자가 누른게 의도한게 아니라면 다시 핸들러를 설정한다.
		longjmp(jump_buffer, 1);//다시 setjmp로 돌아간다.
	}
}	

