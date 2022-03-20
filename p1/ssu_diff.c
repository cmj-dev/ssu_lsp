#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define BUFFER_SIZE 1024

char *filename;
char startfile[BUFFER_SIZE];
char startpath[BUFFER_SIZE];

int filt_fn_reg(const struct dirent *dest);
int filt_fn_dir(const struct dirent *dest);
int filt_reg(const struct dirent *dest);
int filt_dir(const struct dirent *dest);
void get_dir_size(char *path, long *size);
void get_mode(mode_t st_mode, char *mode);
void get_file_list(char *path, char **file_list, int *index, bool is_dir, long exist_size);
void print_file_list(char **file_list, int file_list_length);
void ssu_help(void);
void ssu_find(char *buf);
int which_func(char *buf);
void ssu_diff(char *f1, char *f2);


void ssu_diff(char *f1, char *f2)
{
	off_t *f1_offset;
	off_t *f2_offset;
	FILE *fp1;
	FILE *fp2;
	off_t cur_line;
	char flag;
	char buf1[BUFFER_SIZE];
	char buf2[BUFFER_SIZE];
	int cmp_line = 0;
	int **cns;

	if ((fp1 = fopen(f1, "r")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n", f1);
		exit(1);
	}
	if ((fp2 = fopen(f2, "r")) == NULL)
	{
		fprintf(stderr, "fopen error for %s\n", f2);
		exit(1);
	}
	int line1 = 0;
	int line2 = 0;
	while (fgets(buf1, BUFFER_SIZE, fp1) != NULL)
		line1++;
	while (fgets(buf2, BUFFER_SIZE, fp2) != NULL)
		line2++;
	printf("line1 = %d, line2 = %d\n",line1, line2);
	f1_offset = (off_t *)malloc(sizeof(off_t) * (line1 + 1));
	f2_offset = (off_t *)malloc(sizeof(off_t) * (line2 + 1));
	fseek(fp1, 0, SEEK_SET);
	fseek(fp2, 0, SEEK_SET);
	f1_offset[0] = 0;
	f2_offset[0] = 0;
	f1_offset[1] = 0;
	f2_offset[1] = 0;
	int i1 = 2;
	int i2 = 2;
	while (fgets(buf1, BUFFER_SIZE, fp1) != NULL)
		f1_offset[i1++] = ftell(fp1);
	while (fgets(buf2, BUFFER_SIZE, fp2) != NULL)
		f2_offset[i2++] = ftell(fp2);
	cns = (int **)malloc(sizeof(int *) * (line1 + 1));
	for (int i = 0; i < line1 + 1; i++)
		cns[i] = (int *)malloc(sizeof(int) * (line2 + 1));
	for (int i = 0; i < line1 + 1; i++)
		cns[i][0] = 0;
	for (int i = 0; i < line2 + 1; i++)
		cns[0][i] = 0;
	for (i1 = 1; i1 < line1 + 1; i1++)
	{
		fseek(fp1, f1_offset[i1], SEEK_SET);
		fgets(buf1, BUFFER_SIZE, fp1);
		for (i2 = 1; i2 < line2 + 1; i2++)
		{
			fseek(fp2, f2_offset[i2], SEEK_SET);
			fgets(buf2, BUFFER_SIZE, fp2);
			if (strcmp(buf1, buf2) == 0)
				cns[i1][i2] = cns[i1 - 1][i2 - 1] + 1;
			else
			{
				if (cns[i1 - 1][i2] > cns[i1][i2 - 1])
					cns[i1][i2] = cns[i1 - 1][i2];
				else
					cns[i1][i2] = cns[i1][i2 - 1];
			}
				
		}
	}
	int *result1;
	int *result2;
	result1 = malloc(sizeof(int) * line1 + 1);
	result2 = malloc(sizeof(int) * line2 + 1);
	for (i1 = 1; i1 < line1 + 1; i1++)
		result1[i1] = 0;
	for (i2 = 1; i2 < line2 + 1; i2++)
		result2[i2] = 0;
	
	int max;
	int i = line1;
	int j = line2;
	max = cns[i][j];
	while (cns[i][j] != 0)
	{
		if (cns[i - 1][j] == max)
			i--;
		else if (cns[i][j - 1] == max)
			j--;
		else
		{
			result1[i] = j;
			result2[j] = i;
			i--;
			j--;
			max = cns[i][j];
		}
	}
	for (i1 = 1; i1 < line1 + 1; i1++)
	{
		printf("%d -> %d\n",i1, result1[i1]);
	}
	printf("\n\n");
	for (i2 = 1; i2 < line2 + 1; i2++)
	{
		printf("%d -> %d\n",i2, result2[i2]);
	}
	// int index1_before = 0;
	// int index2_before = 0;
	// while ( )
	// while (result1[i] == 0 && i < line1)
	// 	i++;
	// if (i == line1)
	// {
	// 	if (result1[i] != 0)
	// 	{
	// 		if (result1[i] == line2)
	// 			n;
	// 		else
	// 			a;
	// 	}
	// }
	// if (i - index1_before > 1 && result1[i] - index2_before > 1)
	// 	c;
	// else if (i - index1_before > 1 && result1[i] - index2_before == 1)
	// 	d;
	// else if (i - index1_before == 1 && result1[i] - index2_before > 1)
	// 	a;
}

char *get_str(FILE *fp, off_t offset)
{
	char buf[BUFFER_SIZE];
	fseek(fp, offset, SEEK_SET);
	return (fgets(buf, BUFFER_SIZE, fp));
}