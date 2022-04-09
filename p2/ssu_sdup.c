
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARGMAX 5
#define BUFMAX 1024


int split(char* string, char* seperator, char* argv[]);

int main(void)
{
	int argc = 0;
	char input[BUFMAX];
	char* argv[ARGMAX];

	// 프롬프트 시작
	while (1) {
		printf("20180823> ");
        pid_t pid;
		fgets(input, sizeof(input), stdin);
		input[strlen(input) - 1] = '\0';
		argc = split(input, " ", argv);

		if (argc == 0)
			continue;

        if (!strcmp(argv[0], "fmd5"))
        {
            if ((pid = fork()) == 0)
            {
                execl("fmd5", "fmd5", argv[1], argv[2], argv[3], argv[4], NULL);
            }
            wait(NULL);
        }
        else if (!strcmp(argv[0], "fsha1"))
        {
            if ((pid = fork()) == 0)
            {
                execl("fsha1", "fsha1", argv[1], argv[2], argv[3], argv[4], NULL);
            }
            wait(NULL);
        }
        else if (!strcmp(argv[0], "help"))
        {
            if ((pid = fork()) == 0)
            {
                execl("help", "help", NULL);
            }
            wait(NULL);
        }
        else if (!strcmp(argv[0], "exit"))
        {
            printf("Prompt End\n");
            break;
        }
        else 
        {
			if ((pid = fork()) == 0)
            {
                execl("help", "help", NULL);
            }
            wait(NULL);
        }
    }
	exit(0);
}

int split(char* string, char* seperator, char* argv[])
{
	int argc = 0;
	char* ptr = NULL;

	ptr = strtok(string, seperator);
	while (ptr != NULL) {
		argv[argc++] = ptr;
		ptr = strtok(NULL, seperator);
	}

	return argc;
}