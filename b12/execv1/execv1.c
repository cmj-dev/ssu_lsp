#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void term_stat(int stat);

int main(void)
{
    pid_t pid;
    int status;
    if ((pid = fork()) == 0)
    {
        char *args[] = {"find", "/", "-maxdepth", "4", "-name", "stdio.h", NULL};
        if (execv("/usr/bin/find", args) < 0)
        {// root 디렉토리부터 깊이가 4 이하인 파일 중 name이 stdio.h인 파일 찾기
            fprintf(stderr, "execv error\n");
            exit(1);
        }
    }
    if (wait(&status) == pid)
        term_stat(status);
    else
    {
        fprintf(stderr, "wait error\n");
        exit(1);
    }
    exit(0);
}

void term_stat(int stat)
{
    if (WIFEXITED(stat))//정상종료인 경우
        printf("normally terminated. exit status = %d\n", WEXITSTATUS(stat));
    else if (WIFSIGNALED(stat))//시그널로 인해 종료된 경우
        printf("abnormal termination by signal %d. %s\n", WTERMSIG(stat),
#ifdef WCOREDUMP
               WCOREDUMP(stat) ? "core dumped" : "no core"
#else
               NULL
#endif
        );
    else if (WIFSTOPPED(stat))//프로세스 멈춘 경우
        printf("stopped by signal %d\n", WSTOPSIG(stat));
}