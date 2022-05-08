#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
int main(void)
{
    sigset_t set;//set 이라는 64비트 집합 생성
    sigemptyset(&set);//set 집합 초기화
    sigaddset(&set, SIGINT);//set 집합에 sigint 원소 추가
    switch (sigismember(&set, SIGINT))//sigint가 있는지 확인
    {
    case 1:
        printf("SIGINT is included. \n");//있으므로 include 출력 예상
        break;
    case 0:
        printf("SIGINT is not included. \n");
        break;
    default:
        printf("failed to call sigismember() \n");
    }
    switch (sigismember(&set, SIGSYS))//sigsys가 있는지 확인
    {
    case 1:
        printf("SIGSYS is included. \n");
        break;
    case 0:
        printf("SIGSYS is not included. \n");//없으므로 not included 출력 예상
        break;
    default:
        printf("failed to call sigismember() \n");
    }
    exit(0);
}