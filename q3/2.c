#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define WORD_MAX 100

int main(void)
{
    int fd; 
    int length = 0, offset = 0, count = 0;
    char *fname = "ssu_test.txt";
    char buf[WORD_MAX][BUFFER_SIZE];
    int i;

    if ((fd = open(fname, O_RDONLY)) < 0) {   
        fprintf(stderr, "open error for %s .\n", fname);
        exit(1);
    }
    
    while (read(fd, buf[count], BUFFER_SIZE) > 0)
    {
        int flag = 1;
        for (i = 0; flag; i++)
        {
            if (buf[count][i] == '\n' || buf[count][i] == 0)
            {
                flag = 0;
                buf[count][i] = 0;
                offset = offset + i + 1;
                lseek(fd, offset, SEEK_SET);
                count++;
            }
        }
    }
    close(fd);

    for (i = 0 ; i < count ; i++) 
        printf("%s\n", buf[i]);

    printf("line number : %d \n", count);
    exit(0);
}
