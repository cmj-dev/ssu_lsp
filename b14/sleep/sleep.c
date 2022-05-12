#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void ssu_timestamp(char *str);

int main(void)
{
	unsigned int ret;

	ssu_timestamp("before sleep()");//sleep하기 전 시간을 출력한다.
	ret = sleep(10);
	ssu_timestamp("after sleep()");//10초동안 sleep을 한 후에 다시 시간을 출력한다.
	printf("sleep() returned %d\n", ret);//sleep을 하고 남은 시간을 출력한다.
	exit(0);
}

void ssu_timestamp(char *str)
{
	time_t time_val;

	time(&time_val);
	printf("%s the time is %s\n", str, ctime(&time_val));
}
