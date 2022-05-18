#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

int ssu_daemon_init(void);

int main(void)
{
    pid_t pid;

    pid = getpid();
    printf("parent process : %d\n",pid);
    printf("daemon process initialization\n");

    if (ssu_daemon_init() < 0)
    {
        fprintf(stderr, "ssu_daemon_init failed\n");
        exit(1);
    }
    exit(0);
}

int ssu_daemon_init(void)
{
    pid_t pid;
    int fd, maxfd;

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid != 0)
        exit(0);//daemon의 조건 1번 부모 프로세스를 /etc/init으로 변경해준다.
    
    pid = getpid();
    printf("process %d running as daemon\n", pid);
    setsid();//daemon 조건 2번 새로운 프로세스 그룹을 만든다.
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);//daemon 조건 3번 터미널 입출력 시그널을 무시한다.
    maxfd = getdtablesize();

    for (fd = 0; fd < maxfd; fd++)
        close(fd);//daemon 조건 6번 open되어있는 모든 파일 디스크립터를 닫는다

    umask(0);//daemon 조건 4번 파일의 접근 허가 모드를 모두 허용한다.
    chdir("/");//daemon 조건 5번 현재 디렉토리를 루트 디렉토리로 설정한다.
    fd = open("/dev/null", O_RDWR);
    dup(0);
    dup(0);//daemon 조건 7번 표준 입출력과 에러를 /dev/null로 설정한다.
    return 0;
}