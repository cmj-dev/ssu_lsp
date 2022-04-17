#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void ssu_echo_exit(int status);

int main(void)
{
    pid_t pid;
    int status;

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid == 0)
        exit(7); //첫번째 자식 프로세스 상위 8비트에 7 저장

    if (wait(&status) != pid)
    {
        fprintf(stderr, "wait error\n");
        exit(1);
    }

    ssu_echo_exit(status);//exit(7)로 끝난 자식 프로세스에 대한 status 출력

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid == 0)
        abort();//abort로 시그널 주면서 종료

    if (wait(&status) != pid)
    {
        fprintf(stderr, "wait error\n");
        exit(1);
    }

    ssu_echo_exit(status);//abort로 끝난 자식 프로세스에 대한 status 출력

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid == 0)
        status /= 0 ;

    if (wait(&status) != pid)
    {
        fprintf(stderr, "wait error\n");
        exit(1);
    }

    ssu_echo_exit(status);//0으로 나누기로 끝난 자식 프로세스에 대한 status 출력
    exit(0);
}

void ssu_echo_exit(int status)
{
    if (WIFEXITED(status))
        printf("normal termination, exit status = %d\n", WEXITSTATUS(status));//정상 종료되었을 경우에 exit인자 출력
    else if (WIFSIGNALED(status))//정상종료가 아닐 경우
        printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status),
#ifdef WCOREDUMP
               WCOREDUMP(status) ? " (core file generated)" : "");
#else
               "");
#endif
    else if (WIFSTOPPED(status))//자식프로세스가 현재 중지 상태일 경우
        printf("child stopped, signal number = %d\n", WSTOPSIG(status));
}