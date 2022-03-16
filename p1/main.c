#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 1024

int main(void)
{
	char buf[BUFFER_SIZE];
	char *head = "20180822> ";

	while (1)
	{
		fputs(head, stdout);
		if (fgets(buf, BUFFER_SIZE, stdin) == NULL)
		{
			fprintf(stderr, "error");
			exit(1);
		}
		if (strlen(buf) > 1)
			ssu_help();
	}
	exit(0);
}
