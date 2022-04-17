#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define EXIT_CODE 1
int main(void)
{
    pid_t pid;
    int ret_val, status;
    if ((pid = fork()) == 0)//fork 한 후에 자식 프로세스인 경우 진행
    {
        printf("child: pid = %d ppid = %d exit_code = %d\n",
               getpid(), getppid(), EXIT_CODE);//각각 본인의 프로세스 아이디 부모의 프로세스 아이디를 출력
        exit(EXIT_CODE);
    }
    printf("parent: waiting for child = %d\n", pid);
    ret_val = wait(&status);
    printf("parent: return value = %d, ", ret_val);//종료된 자식 프로세스의 아이디 출력
    printf(" child's status = %x", status);
    printf(" and shifted = %x\n", (status >> 8));//8비트를 shift해서 정상종료인 경우에 exit의 인자를 출력
    exit(0);
}