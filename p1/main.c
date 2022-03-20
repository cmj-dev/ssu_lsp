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
int filt_dir_reg(const struct dirent *dest);
int filt_reg(const struct dirent *dest);
int filt_dir(const struct dirent *dest);
void get_dir_size(char *path, long *size);
void get_mode(mode_t st_mode, char *mode);
void get_file_list(char *path, char **file_list, int *index, bool is_dir, long exist_size);
void get_file_list_leng(char *path, int *index, bool is_dir, long exist_size);
void print_file_list(char **file_list, int file_list_length);
void ssu_help(void);
void ssu_find(char *buf);
int which_func(char *buf);
void ssu_dir_diff(char *f1, char *f2);
void ssu_diff(FILE *fp1, FILE *fp2);
bool ssu_is_diff(FILE *fp1, FILE *fp2);
void print_block(int *block, off_t *f1_offset, off_t *f2_offset, FILE *fp1, FILE *fp2);

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

int filt_fn_reg(const struct dirent *dest)
{
	return ((!strcmp(dest->d_name, filename))&&(dest->d_type == DT_REG));
}

int filt_fn_dir(const struct dirent *dest)
{
	return ((!strcmp(dest->d_name, filename))&&(dest->d_type == DT_DIR));
}

int filt_dir_reg(const struct dirent *dest)
{
	return ((dest->d_type == DT_REG) || ((dest->d_type == DT_DIR)&&(strcmp(dest->d_name,"."))&&(strcmp(dest->d_name,".."))));
}

int filt_dir(const struct dirent *dest)
{
	return((dest->d_type == DT_DIR)&&(strcmp(dest->d_name,"."))&&(strcmp(dest->d_name,"..")));
}

int filt_reg(const struct dirent *dest)
{
	return((dest->d_type == DT_REG));
}

void get_dir_size(char *path, long *size)
{
	struct dirent **dir_res;
	struct dirent **file_res;
	struct stat dir_stat;
	int dir_num;
	int fn_num;
	char temp[BUFFER_SIZE];

	fn_num = scandir(path, &file_res, filt_reg, NULL);
	for (int i = 0; i < fn_num; i++)
	{
		strcpy(temp, path);
		stat(strcat(strcat(temp,"/"),file_res[i]->d_name),&dir_stat);
		*size += dir_stat.st_size;
		free(file_res[i]);
	}
	
	dir_num = scandir(path, &dir_res, filt_dir, alphasort);
	for (int i = 0; i < dir_num; i++)
	{
		strcpy(temp, path);
		if (!strcmp(temp,"/"))
			strcat(temp,dir_res[i]->d_name);
		else
			strcat(strcat(temp,"/"),dir_res[i]->d_name);
		get_dir_size(temp, size);
		free(dir_res[i]);
	}
	free(file_res);
	free(dir_res);
}

void get_mode(mode_t st_mode, char *mode)
{
	for (int i = 0; i < 9; i++)
	{
		if (st_mode % 2 == 0)
			mode[9-i] = '-';
		else
			mode[9-i] = '+';
		st_mode /= 2;
	}
	for (int i = 1; i < 10; i++)
	{
		if (i % 3 == 1 && mode[i] == '+')
			mode[i] = 'r';
		else if (i % 3 == 2 && mode[i] == '+')
			mode[i] = 'w';
		else if (i % 3 == 0 && mode[i] == '+')
			mode[i] = 'x';
	}
}

void get_file_list(char *path, char **file_list, int *index, bool is_dir, long exist_size)
{
	struct dirent **dir_res;
	struct dirent **fn_res;
	int dir_num;
	int fn_num;
	long size = 0;
	char temp[BUFFER_SIZE];

	if (is_dir)
		fn_num = scandir(path, &fn_res, filt_fn_dir, alphasort);
	else
		fn_num = scandir(path, &fn_res, filt_fn_reg, alphasort);
	for (int i = 0; i < fn_num; i++)
	{
		strcpy(temp, path);
		struct stat temp_stat;

		stat(strcat(strcat(temp, "/"), fn_res[i]->d_name), &temp_stat);
		if (is_dir)
		{
			get_dir_size(temp, &size);
			if (exist_size == size && (strcmp(startfile, temp) != 0))
				strcpy(file_list[(*index)++], temp);
		}
		else if (exist_size == temp_stat.st_size && (strcmp(startfile, temp) != 0))
			strcpy(file_list[(*index)++], temp);
		free(fn_res[i]);
	}
	dir_num = scandir(path, &dir_res, filt_dir, alphasort);
	for (int i = 0; i < dir_num; i++)
	{
		strcpy(temp, path);
		if (!strcmp(temp,"/"))
			strcat(temp,dir_res[i]->d_name);
		else
			strcat(strcat(temp,"/"),dir_res[i]->d_name);
		if (strcmp(temp,"/run")&&strcmp(temp,"/proc")&&strcmp(temp,"/mnt/c"))
			get_file_list(temp, file_list, index, is_dir, exist_size);
		free(dir_res[i]);
	}
	free(fn_res);
	free(dir_res);
}

void get_file_list_leng(char *path, int *index, bool is_dir, long exist_size)
{
	struct dirent **dir_res;
	struct dirent **fn_res;
	int dir_num;
	int fn_num;
	long size = 0;
	char temp[BUFFER_SIZE];

	if (is_dir)
		fn_num = scandir(path, &fn_res, filt_fn_dir, alphasort);
	else
		fn_num = scandir(path, &fn_res, filt_fn_reg, alphasort);
	for (int i = 0; i < fn_num; i++)
	{
		strcpy(temp, path);
		struct stat temp_stat;

		stat(strcat(strcat(temp, "/"), fn_res[i]->d_name), &temp_stat);
		free(fn_res[i]);
		if (is_dir)
		{
			get_dir_size(temp, &size);
			if (exist_size == size && (strcmp(startfile, temp) != 0))
				(*index)++;
		}
		else if (exist_size == temp_stat.st_size && (strcmp(startfile, temp) != 0))
			(*index)++;
	}
	dir_num = scandir(path, &dir_res, filt_dir, alphasort);
	for (int i = 0; i < dir_num; i++)
	{
		strcpy(temp, path);
		if (!strcmp(temp,"/"))
			strcat(temp,dir_res[i]->d_name);
		else
			strcat(strcat(temp,"/"),dir_res[i]->d_name);
		free(dir_res[i]);
		if (strcmp(temp,"/run")&&strcmp(temp,"/proc")&&strcmp(temp,"/mnt/c"))
			get_file_list_leng(temp, index, is_dir, exist_size);
	}
	free(fn_res);
	free(dir_res);
}

void print_file_list(char **file_list, int file_list_length)
{
	int i = 0;
	long size = 0;
	char mode[11];
	struct stat print_stat;
	
	printf("Index Size Mode\t\tBlocks Links UID\tGID\tAccess\t\tChange\t\tModify\t\tPath\n");
	for (i = 0; i < file_list_length; i++)
	{
		size = 0;
		stat(file_list[i], &print_stat);
		if ((S_IFMT & print_stat.st_mode) == S_IFDIR)
		{
			mode[0] = 'd';
			get_dir_size(file_list[i], &size);
		}
		else
		{
			mode[0] = '-';
			size = print_stat.st_size;
		}
		get_mode(print_stat.st_mode, mode);
		//index size mode blocks links uid gid access change modify path
		printf("%-5d %-4ld %s\t%-6ld %-5ld %d\t%d\t%2d-%02d-%02d %02d:%02d\t%2d-%02d-%02d %02d:%02d\t%2d-%02d-%02d %02d:%02d\t%s\n",
			   i, size, mode, print_stat.st_blocks, print_stat.st_nlink,
			   print_stat.st_uid, print_stat.st_gid,
			   localtime(&print_stat.st_atim.tv_sec)->tm_year - 100, localtime(&print_stat.st_atim.tv_sec)->tm_mon + 1, localtime(&print_stat.st_atim.tv_sec)->tm_mday,
			   localtime(&print_stat.st_atim.tv_sec)->tm_hour, localtime(&print_stat.st_atim.tv_sec)->tm_min,
			   localtime(&print_stat.st_ctim.tv_sec)->tm_year - 100, localtime(&print_stat.st_ctim.tv_sec)->tm_mon + 1, localtime(&print_stat.st_ctim.tv_sec)->tm_mday,
			   localtime(&print_stat.st_ctim.tv_sec)->tm_hour, localtime(&print_stat.st_ctim.tv_sec)->tm_min,
			   localtime(&print_stat.st_mtim.tv_sec)->tm_year - 100, localtime(&print_stat.st_mtim.tv_sec)->tm_mon + 1, localtime(&print_stat.st_mtim.tv_sec)->tm_mday,
			   localtime(&print_stat.st_mtim.tv_sec)->tm_hour, localtime(&print_stat.st_mtim.tv_sec)->tm_min,
			   file_list[i]);
	}
}

void ssu_help(void)
{
    printf("Usage:\n\t> find [FILENAME] [PATH]\n\t\t>> [INDEX] [OPTION ... ]\n\t> help\n\t> exit\n");
}

void ssu_find(char *buf)
{
	char *head = ">> ";
	char *path;
	char filetemp[BUFFER_SIZE];
	char temp[BUFFER_SIZE];
	char *dump;
	char **file_list;
	bool is_dir = false;
	bool valid = true;
	struct stat file;
	long size = 0;
	int file_list_length = 0;
	char *sub_prompt;
	char *index;
	char *option;
	int i_index;
	FILE *fp1;
	FILE *fp2;

	buf[strlen(buf) - 1] = 0;
	filename = strtok(NULL, " ");
	path = strtok(NULL, " ");
	dump = strtok(NULL, " ");
	if (filename == NULL)
	{
		printf("Usage:> find [FILENAME] [PATH]\n");
		return ;
	}	
	else if (path == NULL)
	{
		printf("Usage:> find [FILENAME] [PATH]\n");
		return ;
	}
	else if (dump != NULL)
	{
		printf("Usage:> find [FILENAME] [PATH]\n");
		return ;
	}
	
	else
	{
		realpath(filename, filetemp);
		int i = 0;
		int j = 0;
		bool flag = false;
		while (filename[i] != '\0')
		{
			if (filename[i] == '/')
			{
				j = i;
				flag = true;
			}
			i++;
		}
		if (flag)
			strcpy(filename, &filename[j + 1]);
		realpath(path, temp);
		strcpy(startfile, filetemp);
		strcpy(startpath, temp);
		if (stat(startpath, &file) < 0)
		{
			fprintf(stderr, "not such PATH [%s]\n",startpath);
			return ;
		}
		if (stat(startfile, &file) < 0)
		{
			fprintf(stderr, "not such FILENAME [%s]\n",startfile);
			return ;
		}
		if ((file.st_mode & S_IFMT) == S_IFDIR)
		{
			is_dir = true;
			get_dir_size(startfile, &size);
		}
		else
		{
			is_dir = false;
			size = file.st_size;
		}
		get_file_list_leng(startpath, &file_list_length, is_dir, size);
		file_list = (char **)malloc(sizeof(char *) * (file_list_length + 1));
		for (int i = 0; i < file_list_length + 1; i++)
			file_list[i] = (char *)malloc(sizeof(char) * BUFFER_SIZE);
		strcpy(file_list[0], startfile);
		file_list_length = 1;
		get_file_list(startpath, file_list, &file_list_length, is_dir, size);
		print_file_list(file_list, file_list_length);
		if (file_list_length == 1)
		{
			printf("(None)\n");
			for (i = 0; i < file_list_length; i++)
				free(file_list[i]);
			free(file_list);
			return ;
		}
		while (1)
		{
			dump = NULL;
			fputs(head, stdout);
			if ((sub_prompt = fgets(buf, BUFFER_SIZE, stdin)) == NULL) // get line by line
			{
				fprintf(stderr, "fgets error\n");
				exit(1);
			}
			if (strlen(sub_prompt) < 2)
				printf("Usage : [INDEX] [OPTION]\n");
			else
			{
				sub_prompt[strlen(sub_prompt) - 1] = ' ';
				index = strtok(sub_prompt, " ");
				option = strtok(NULL, " ");
				dump = strtok(NULL, " ");
				if (dump != NULL)
					printf("Usage : [INDEX] [OPTION]\n");
				else
				{
					int i = 0;
					valid = true;
					while (index[i] != '\0')
					{
						if (index[i] < '0' || index[i] > '9')
						{
							printf("Usage : [INDEX] [OPTION]\n");
							valid = false;
							break ;
						}
						i++;
					}
					if (valid)
					{
						i_index = atoi(index);
						if (i_index >= file_list_length || i_index < 1)
						{
							printf("Please enter num between 1 ~ %d\n", file_list_length - 1);
							valid = false;
						}
					}
					if (valid)
					{
						if (is_dir)
						{
							char zero_list[BUFFER_SIZE];
							char index_list[BUFFER_SIZE];
							strcpy(zero_list,file_list[0]);
							strcpy(index_list,file_list[i_index]);
							for (i = 0; i < file_list_length; i++)
								free(file_list[i]);
							free(file_list);
							ssu_dir_diff(zero_list, index_list);
							break;
						}
						else
						{
							char zero_list[BUFFER_SIZE];
							char index_list[BUFFER_SIZE];
							strcpy(zero_list,file_list[0]);
							strcpy(index_list,file_list[i_index]);
							for (i = 0; i < file_list_length; i++)
								free(file_list[i]);
							free(file_list);
							if ((fp1 = fopen(zero_list, "r")) == NULL)
								fprintf(stderr, "fopen error");
							if ((fp2 = fopen(index_list, "r")) == NULL)
								fprintf(stderr, "fopen error");
							ssu_diff(fp1, fp2);
							fclose(fp1);
							fclose(fp2);
							break;
						}
					}
				}
			}
		}
	}
}

void ssu_dir_diff(char *f1, char *f2)
{
	struct dirent **dir1;
	struct dirent **dir2;
	int ndir1;
	int ndir2;
	int i;
	int j;
	int index;
	char diff1[BUFFER_SIZE];
	char diff2[BUFFER_SIZE];
	FILE *fp1;
	FILE *fp2;

	i = 0;
	index = 0;
	while (f1[i] == f2[i])
	{
		if (f1[i] == '/')
			index = i;
		i++;
	}
	i = 0;
	strcpy(diff1, &f1[index + 1]);
	strcpy(diff2, &f2[index + 1]);
	i = 0;
	j = 0;
	index = 0;
	ndir1 = scandir(f1, &dir1, filt_dir_reg, alphasort);
	ndir2 = scandir(f2, &dir2, filt_dir_reg, alphasort);
	while (i < ndir1)
	{
		j = index;
		while (j < ndir2)
		{
			if (!strcmp(dir1[i]->d_name, dir2[j]->d_name))
			{
				for (int in = index; in < j; in++)
				{
					printf("Only in %s: %s\n", diff2, dir2[in]->d_name);
				}
				index = j + 1;
				if (dir1[i]->d_type != dir2[j]->d_type)
				{
					if (dir1[i]->d_type == DT_DIR)
						printf("File %s is a directory while file %s is a regular file\n",
							   strcat(strcat(strndup(diff1, BUFFER_SIZE), "/"), dir1[i]->d_name), strcat(strcat(strndup(diff2, BUFFER_SIZE), "/"), dir2[j]->d_name));
					else
						printf("File %s is a regular file while file %s is a directory\n",
							   strcat(strcat(strndup(diff1, BUFFER_SIZE), "/"), dir1[i]->d_name), strcat(strcat(strndup(diff2, BUFFER_SIZE), "/"), dir2[j]->d_name));
				}
				else
				{
					if (dir1[i]->d_type == DT_DIR)
						printf("Common subdirectories: %s and %s\n",
							   strcat(strcat(strndup(diff1, BUFFER_SIZE), "/"), dir1[i]->d_name), strcat(strcat(strndup(diff2, BUFFER_SIZE), "/"), dir2[j]->d_name));
					else
					{
						if ((fp1 = fopen(strcat(strcat(strndup(f1, BUFFER_SIZE), "/"), dir1[i]->d_name), "r")) == NULL)
							fprintf(stderr,"fopen error\n");
						if ((fp2 = fopen(strcat(strcat(strndup(f2, BUFFER_SIZE), "/"), dir2[i]->d_name), "r")) == NULL)
							fprintf(stderr,"fopen error\n");
						if (ssu_is_diff(fp1, fp2))
						{
							fclose(fp1);
							fclose(fp2);
							printf("diff %s %s\n",
								   strcat(strcat(strndup(diff1, BUFFER_SIZE), "/"), dir1[i]->d_name), strcat(strcat(strndup(diff2, BUFFER_SIZE), "/"), dir2[j]->d_name));
							if ((fp1 = fopen(strcat(strcat(strndup(f1, BUFFER_SIZE), "/"), dir1[i]->d_name), "r")) == NULL)
								fprintf(stderr, "fopen error\n");
							if ((fp1 = fopen(strcat(strcat(strndup(f2, BUFFER_SIZE), "/"), dir2[i]->d_name), "r")) == NULL)
								fprintf(stderr, "fopen error\n");
							ssu_diff(fp1, fp2);
						}
						fclose(fp1);
						fclose(fp2);
					}
				}
				break;
			}
			j++;
		}
		if (j == ndir2)
			printf("Only in %s: %s\n", diff1, dir1[i]->d_name);
		i++;
	}
	for (int in = index; in < ndir2; in++)
	{
		printf("Only in %s: %s\n", diff2, dir2[in]->d_name);
	}

	for (i = 0; i < ndir1; i++)
		free(dir1[i]);
	for (j = 0; j < ndir2; j++)
		free(dir2[j]);
	free(dir1);
	free(dir2);
}

void ssu_diff(FILE *fp1, FILE *fp2)
{
	off_t *f1_offset;
	off_t *f2_offset;
	char buf1[BUFFER_SIZE];
	char buf2[BUFFER_SIZE];
	int line1 = 0;
	int line2 = 0;
	while (fgets(buf1, BUFFER_SIZE, fp1) != NULL)
		line1++;
	while (fgets(buf2, BUFFER_SIZE, fp2) != NULL)
		line2++;
	f1_offset = (off_t *)malloc(sizeof(off_t) * (line1 + 2));
	f2_offset = (off_t *)malloc(sizeof(off_t) * (line2 + 2));
	fseek(fp1, 0, SEEK_SET);
	fseek(fp2, 0, SEEK_SET);
	f1_offset[0] = 0;
	f2_offset[0] = 0;
	f1_offset[1] = 0;
	f2_offset[1] = 0;
	int i = 2;
	while (fgets(buf1, BUFFER_SIZE, fp1) != NULL)
		f1_offset[i++] = ftell(fp1);
	i = 2;
	while (fgets(buf2, BUFFER_SIZE, fp2) != NULL)
		f2_offset[i++] = ftell(fp2);
	fseek(fp1, -1, SEEK_END);
	fseek(fp2, -1, SEEK_END);
	if (getc(fp1) == '\n')
		f1_offset[line1 + 1] = 0;
	else
		f1_offset[line1 + 1] = -1;
	if (getc(fp2) == '\n')
		f2_offset[line2 + 1] = 0;
	else
		f2_offset[line2 + 1] = -1;
	int result1[BUFFER_SIZE];
	int result2[BUFFER_SIZE];
	i = 1;
	int j = 1;
	int start = 1;
	bool line_before_is_n = false;
	while (i < line1 + 1)
	{
		j = start;
		fseek(fp1, f1_offset[i], SEEK_SET);
		fgets(buf1, BUFFER_SIZE, fp1);
		while (j < line2 + 1)
		{
			fseek(fp2, f2_offset[j], SEEK_SET);
			fgets(buf2, BUFFER_SIZE, fp2);
			if (strcmp(buf1, buf2) == 0)
			{
				result1[i] = j;
				if (line_before_is_n)
				{
					if (result1[i] < result1[i - 1])
						result1[i-1] = 0;
					else
					{
						fseek(fp2, f2_offset[result1[i] - 1], SEEK_SET);
						fgets(buf2,BUFFER_SIZE,fp2);
						if (strcmp(buf2,"\n") == 0)
							result1[i - 1] = result1[i] - 1;
					}
				}
				if (strcmp(buf1, "\n"))
				{
					line_before_is_n = false;
					start = j + 1;
				}
				else
					line_before_is_n = true;
				break;
			}
			j++;
		}
		if (j == line2 + 1)
			result1[i] = 0;
		i++;
	}
	i = 1;
	j = 1;
	start = 1;
	line_before_is_n = false;
	while (i < line2 + 1)
	{
		j = start;
		fseek(fp2, f2_offset[i], SEEK_SET);
		fgets(buf2, BUFFER_SIZE, fp2);
		while (j < line1 + 1)
		{
			fseek(fp1, f1_offset[j], SEEK_SET);
			fgets(buf1, BUFFER_SIZE, fp1);
			if (strcmp(buf1, buf2) == 0)
			{
				result2[i] = j;
				if (line_before_is_n)
				{
					if (result2[i] < result2[i - 1])
						result2[i - 1] = 0;
					else
					{
						fseek(fp1, f1_offset[result2[i] - 1], SEEK_SET);
						fgets(buf1, BUFFER_SIZE, fp1);
						if (strcmp(buf1, "\n") == 0)
							result2[i - 1] = result2[i] - 1;
					}
				}
				if (strcmp(buf2, "\n"))
				{
					line_before_is_n = false;
					start = j + 1;
				}
				else
					line_before_is_n = true;
				break;
			}
			j++;
		}
		if (j == line1 + 1)
			result2[i] = 0;
		i++;
	}
	int ressum1 = 0;
	int ressum2 = 0;
	for (i = 1; i < line1 + 1; i++)
	{
		if (result1[i] == 0)
			ressum1++;
	}
	for (i = 1; i < line2 + 1; i++)
	{
		if (result2[i] == 0)
			ressum2++;
	}
	if (ressum1 > ressum2)
	{
		for (i = 1; i < line1 + 1; i++)
			result1[i] = 0;
		for (i = 1; i < line2 + 1; i++)
		{
			if (result2[i] != 0)
				result1[result2[i]] = i;
		}
	}
	int block[4] = {0, 0, 0, 0};
	i = 1;
	while (i < line1 + 1)
	{
		if (result1[i] != 0)
		{
			block[0] = block[1];
			block[2] = block[3];
			block[1] = i;
			block[3] = result1[i];
			print_block(block, f1_offset, f2_offset, fp1, fp2);
		}
		i++;
	}
	block[0] = block[1];
	block[2] = block[3];
	block[1] = line1 + 1;
	block[3] = line2 + 1;
	print_block(block, f1_offset, f2_offset, fp1, fp2);
	free(f1_offset);
	free(f2_offset);
}

bool ssu_is_diff(FILE *fp1, FILE *fp2)
{
	off_t *f1_offset;
	off_t *f2_offset;
	char buf1[BUFFER_SIZE];
	char buf2[BUFFER_SIZE];
	int line1 = 0;
	int line2 = 0;
	fseek(fp1, 0, SEEK_SET);
	fseek(fp2, 0, SEEK_SET);
	while (fgets(buf1, BUFFER_SIZE, fp1) != NULL)
		line1++;
	while (fgets(buf2, BUFFER_SIZE, fp2) != NULL)
		line2++;
	f1_offset = (off_t *)malloc(sizeof(off_t) * (line1 + 2));
	f2_offset = (off_t *)malloc(sizeof(off_t) * (line2 + 2));
	fseek(fp1, 0, SEEK_SET);
	fseek(fp2, 0, SEEK_SET);
	f1_offset[0] = 0;
	f2_offset[0] = 0;
	f1_offset[1] = 0;
	f2_offset[1] = 0;
	int i = 2;
	while (fgets(buf1, BUFFER_SIZE, fp1) != NULL)
		f1_offset[i++] = ftell(fp1);
	i = 2;
	while (fgets(buf2, BUFFER_SIZE, fp2) != NULL)
		f2_offset[i++] = ftell(fp2);
	fseek(fp1, -1, SEEK_END);
	fseek(fp2, -1, SEEK_END);
	if (getc(fp1) == '\n')
		f1_offset[line1 + 1] = 0;
	else
		f1_offset[line1 + 1] = -1;
	if (getc(fp2) == '\n')
		f2_offset[line2 + 1] = 0;
	else
		f2_offset[line2 + 1] = -1;
	int result1[BUFFER_SIZE];
	int result2[BUFFER_SIZE];
	i = 1;
	int j = 1;
	int start = 1;
	bool line_before_is_n = false;
	while (i < line1 + 1)
	{
		j = start;
		fseek(fp1, f1_offset[i], SEEK_SET);
		fgets(buf1, BUFFER_SIZE, fp1);
		while (j < line2 + 1)
		{
			fseek(fp2, f2_offset[j], SEEK_SET);
			fgets(buf2, BUFFER_SIZE, fp2);
			if (strcmp(buf1, buf2) == 0)
			{
				result1[i] = j;
				if (line_before_is_n)
				{
					if (result1[i] < result1[i - 1])
						result1[i-1] = 0;
					else
					{
						fseek(fp2, f2_offset[result1[i] - 1], SEEK_SET);
						fgets(buf2,BUFFER_SIZE,fp2);
						if (strcmp(buf2,"\n") == 0)
							result1[i - 1] = result1[i] - 1;
					}
				}
				if (strcmp(buf1, "\n"))
				{
					line_before_is_n = false;
					start = j + 1;
				}
				else
					line_before_is_n = true;
				break;
			}
			j++;
		}
		if (j == line2 + 1)
			result1[i] = 0;
		i++;
	}
	i = 1;
	j = 1;
	start = 1;
	line_before_is_n = false;
	while (i < line2 + 1)
	{
		j = start;
		fseek(fp2, f2_offset[i], SEEK_SET);
		fgets(buf2, BUFFER_SIZE, fp2);
		while (j < line1 + 1)
		{
			fseek(fp1, f1_offset[j], SEEK_SET);
			fgets(buf1, BUFFER_SIZE, fp1);
			if (strcmp(buf1, buf2) == 0)
			{
				result2[i] = j;
				if (line_before_is_n)
				{
					if (result2[i] < result2[i - 1])
						result2[i - 1] = 0;
					else
					{
						fseek(fp1, f1_offset[result2[i] - 1], SEEK_SET);
						fgets(buf1, BUFFER_SIZE, fp1);
						if (strcmp(buf1, "\n") == 0)
							result2[i - 1] = result2[i] - 1;
					}
				}
				if (strcmp(buf2, "\n"))
				{
					line_before_is_n = false;
					start = j + 1;
				}
				else
					line_before_is_n = true;
				break;
			}
			j++;
		}
		if (j == line1 + 1)
			result2[i] = 0;
		i++;
	}
	int ressum1 = 0;
	int ressum2 = 0;
	for (i = 1; i < line1 + 1; i++)
	{
		if (result1[i] == 0)
			ressum1++;
	}
	for (i = 1; i < line2 + 1; i++)
	{
		if (result2[i] == 0)
			ressum2++;
	}
	free(f1_offset);
	free(f2_offset);
	
	if (ressum1 == 0 || ressum2 == 0)
		return (false);
	else
		return (true);	
}

void print_block(int *block, off_t *f1_offset, off_t *f2_offset, FILE *fp1, FILE *fp2)
{
	int i;
	char buf[BUFFER_SIZE];
	if ((block[1] - block[0]) == 1 && (block[3] - block[2]) == 1)
		return;
	if ((block[0] + 1) == block[1])
	{
		if ((block[3] - block[2]) == 2)
			printf("%da%d\n", block[0], block[2] + 1);
		else
			printf("%da%d,%d\n", block[0], block[2] + 1, block[3] - 1);
		i = block[2] + 1;
		while (i < block[3])
		{
			fseek(fp2, f2_offset[i], SEEK_SET);
			printf("> %s", fgets(buf, BUFFER_SIZE, fp2));
			i++;
		}
		if (f2_offset[block[3]] < 0)
			printf("\n\\ No newline at end of file\n");
	}
	else if ((block[2] + 1) == block[3])
	{
		if ((block[1] - block[0]) == 2)
			printf("%dd%d", block[0] + 1, block[2]);
		printf("%d,%dd%d\n", block[0] + 1, block[1] - 1, block[2]);
		i = block[0] + 1;
		while (i < block[1])
		{
			fseek(fp1, f1_offset[i], SEEK_SET);
			printf("< %s", fgets(buf, BUFFER_SIZE, fp1));
			i++;
		}
		if (f1_offset[block[1]] < 0)
			printf("\n\\ No newline at end of file\n");
	}
	else
	{
		if ((block[1] - block[0]) == 2 && (block[3] - block[2]) == 2)
			printf("%dc%d\n", block[0] + 1, block[2] + 1);
		else if ((block[3] - block[2]) == 2)
			printf("%d,%dc%d\n", block[0] + 1, block[1] - 1, block[3] - 1);
		else if ((block[1] - block[0]) == 2)
			printf("%dc%d,%d\n", block[0] + 1, block[2] + 1, block[3] - 1);
		else
			printf("%d,%dc%d,%d\n", block[0] + 1, block[1] - 1, block[2] + 1, block[3] - 1);
		i = block[0] + 1;
		while (i < block[1])
		{
			fseek(fp1, f1_offset[i], SEEK_SET);
			printf("< %s", fgets(buf, BUFFER_SIZE, fp1));
			i++;
		}
		if (f1_offset[block[1]] < 0)
			printf("\n\\ No newline at end of file\n");
		printf("---\n");
		i = block[2] + 1;
		while (i < block[3])
		{
			fseek(fp2, f2_offset[i], SEEK_SET);
			printf("> %s", fgets(buf, BUFFER_SIZE, fp2));
			i++;
		}
		if (f2_offset[block[3]] < 0)
			printf("\n\\ No newline at end of file\n");
	}
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