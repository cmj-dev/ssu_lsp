#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(void)
{
    if (fork() == 0)
        execl("/bin/echo", "echo", "this is", "message one", (char *)0);//linux 기본 명령어 echo 진행 마지막에는 널 포인터 추가.
    if (fork() == 0)
        execl("/bin/echo", "echo", "this is", "message Two", (char *)0);
    printf("parent: waiting for children\n");
    while (wait((int *)0) != -1)//자식 프로세스가 없을때 까지 진행
        ;
    printf("parent: all children terminated\n");
    exit(0);
}