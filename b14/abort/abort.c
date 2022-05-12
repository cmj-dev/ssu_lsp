#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	printf("abort terminate this program\n");
	abort();
	printf("this line is never reached\n");//abort에서 프로그램이 종료되기 때문에 이 문장은 실행되지 않는다.
	exit(0);
}
