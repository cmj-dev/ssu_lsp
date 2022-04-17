
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(void)
{
    printf("this is the original program\n");
    execl("./ssu_execl_test_1", "ssu_execl_test_1", "param1", "param2", "param3", (char *)0);
    printf("%s\n", "this line should never get printed\n");//새로운 프로세스가 대신 실행되었으므로 프린트되지 않는다.
    exit(0);
}