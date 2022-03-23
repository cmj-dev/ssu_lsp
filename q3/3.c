#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 128
   
int main(int argc, char *argv[])
{
   char buf[BUFFER_SIZE];
   int fd1, fd2;
   ssize_t size;
   
   if (argc != 3)
   {
       fprintf(stderr, "usage : %s <src file> <dest file>\n", argv[0]);
       exit(1);
   }
   if ((fd1 = open(argv[1], O_RDONLY)) < 0)
   {
       fprintf(stderr, "%s open error\n", argv[1]);
       exit(1);
   }
   if ((fd2 = open(argv[2], O_CREAT | O_RDWR | O_TRUNC, 0644)) < 0)
   {
       fprintf(stderr, "%s open error\n", argv[1]);
       exit(1);
   }
   size = read(fd1, buf, BUFFER_SIZE);
   write(fd2, buf, size);
}