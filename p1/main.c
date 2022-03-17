#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024

void ssu_help(void)
{
    printf("Usage:\n\t> find [FILENAME] [PATH]\n\t\t>> [INDEX] [OPTION ... ]\n\t> help\n\t> exit\n");
}

void ssu_find(char *buf)
{
	char *filename;
	char *path;

	buf[strlen(buf) - 1] = 0;
	filename = strtok(NULL, " ");
	path = strtok(NULL, " ");
	if (filename == NULL)
		printf("please enter FILENAME\n");
	else if (path == NULL)
		printf("please enter PATH\n");
	else
		printf("%s, %s\n",filename, path);
}

int which_func(char *buf)
{
	char *a;

	buf[strlen(buf) - 1] = 0;
	a = strtok(buf, " ");
	if (strcmp(a, "find") == 0)
		return (0);
	else if (strcmp(a, "help") == 0)
		return (1);
	else if (strcmp(a, "exit") == 0)
		return (2);
	else
		return (1);
}
int main(void)
{
	char buf[BUFFER_SIZE];
	char *head = "20180822> ";
	int i;
	struct timeval starttime;
	struct timeval endtime;
	if (gettimeofday(&starttime, NULL) < 0)
	{
		fprintf(stderr, "gettimeofday error\n");
		exit(1);
	} //get start time for exit function
	while (1)
	{
		fputs(head, stdout);
		if (fgets(buf, BUFFER_SIZE, stdin) == NULL) // get line by line
		{
			fprintf(stderr, "fgets error\n");
			exit(1);
		}
		if (strlen(buf) > 1)
		{
			i = which_func(buf);
			if (i == 1)//for help function
				ssu_help();
			else if (i == 2)//for exit function
			{
				if (gettimeofday(&endtime, NULL) < 0)
				{
					fprintf(stderr, "gettimeofday error\n");
					exit(1);
				}
				printf("Prompt End\n");
				if (endtime.tv_usec < starttime.tv_usec)
					printf("Runtime: %ld:%ld(sec:usec)\n",endtime.tv_sec - starttime.tv_sec - 1, 1000000 + endtime.tv_usec - starttime.tv_usec);
				else
					printf("Runtime: %ld:%ld(sec:usec)\n",endtime.tv_sec - starttime.tv_sec, endtime.tv_usec - starttime.tv_usec);
				exit(0);
			}
			else if (i == 0)//for find function
			{
				ssu_find(buf);
			}
		}
	}
}
