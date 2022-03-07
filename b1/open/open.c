#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int	main(void)
{
	int		fd;
	char	*file_name;

	file_name = "ssu_test.txt";
	if ((fd = open(file_name, O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n", file_name);
		exit(1);
	}
	else
		printf("Success!\nFilename : %s\nDescriptor : %d\n", file_name, fd);
	exit(0);
}
