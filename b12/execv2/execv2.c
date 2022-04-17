#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(void)
{
    char *argv[] = {
        "ssu_execl_test_1", "param1", "param2", (char *)0};
    printf("this is the original program\n");
    execv("./ssu_execl_test_1", argv);//해당 프로세스가 ssu_execl_test_1으로 대체됨.
    printf("%s\n", "This line should never get printed\n");//대체되었기 때문에 프린트되지 않음
    exit(0);
}