#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/md5.h>

#define ARGMAX 5
#define SUB_ARGMAX 3
#define FILEMAX 255
#define PATHMAX 4096
#define BUFMAX 1024
#define BUFSIZE 1024 * 16

struct lower_list{
    char path[PATHMAX];
    struct lower_list *next;
};

struct file_list{
    unsigned char md[MD5_DIGEST_LENGTH];
    char path[PATHMAX];
    long size;
    struct file_list *next;
    struct lower_list *l_start;
};

struct file_list *start;
char extension[FILEMAX];
long lminsize;
long lmaxsize;
char target_dir[PATHMAX];

void command_sub(void);
void do_fp(char *path, unsigned char *md);
void search_src_file(char *dirname);
void search_file(char *dirname, struct file_list *node);
void pt(unsigned char *md);
void add_node(struct file_list *new);
void create_new_node(struct file_list *new, unsigned char *md, char *path, long size);
void add_low_node(struct lower_list *l_start, char *path);
bool hashcmp(unsigned char *dest, unsigned char *src);
void filter_list(void);
bool have_path(char *path);
void print_result(void);
void rearr_list(void);
void print_time(time_t tv_sec);
int split(char* string, char* seperator, char* argv[]);

void command_fmd5(int argc, char **argv)
{
    double minsize;
    double maxsize;
    char rpath[PATHMAX];
    char path[PATHMAX];
    
    if (argc != ARGMAX)
    {
        printf("ERROR: Arguments error\n");
        return;
    }
    if (!strcmp(argv[1], "*"))
        strcpy(extension, "");
    else if (!strncmp(argv[1], "*.", 2))
        strcpy(extension, &argv[1][1]);
    else
    {
        printf("please enter the correct extension\n");
        return;
    }

    if (strcmp(argv[2],"~"))
    {
        minsize = atof(argv[2]);
        if (strstr(argv[2],"MB") != NULL || strstr(argv[2],"mb") != NULL)
            minsize *= 1000000;
        else if (strstr(argv[2],"GB") != NULL || strstr(argv[2],"gb") != NULL)
            minsize *= 1000000000;
        else if (strstr(argv[2],"KB") != NULL || strstr(argv[2], "kb") != NULL)
            minsize *= 1000;
    }
    else
        minsize = -1;
    if (strcmp(argv[3],"~"))
    {
        maxsize = atof(argv[3]);
        if (strstr(argv[3],"MB") != NULL || strstr(argv[3],"mb") != NULL)
            maxsize *= 1000000;
        else if (strstr(argv[3],"GB") != NULL || strstr(argv[3],"gb") != NULL)
            maxsize *= 1000000000;
        else if (strstr(argv[3],"KB") != NULL || strstr(argv[3], "kb") != NULL)
            maxsize *= 1000;
    }
    else
        maxsize = -1;
    if (!strncmp(argv[4], "~", 1))
    {
        strcpy(path, getenv("HOME"));
        strcat(path, strstr(argv[4], "~") + 1);
    }
    else
        strcpy(path, argv[4]);
    
    if (realpath(path, rpath) == NULL)
    {
        fprintf(stderr, "targetpass error \n");
        return;
    }
    lminsize = (long)minsize;
    lmaxsize = (long)maxsize;
    start = (struct file_list*)malloc(sizeof(struct file_list));
    strcpy(target_dir, rpath);
    struct timeval starttime;
	struct timeval endtime;
	if (gettimeofday(&starttime, NULL) < 0)
	{
		fprintf(stderr, "gettimeofday error\n");
		exit(1);
	}
    search_src_file(rpath);
    if (gettimeofday(&endtime, NULL) < 0) // get endtime
    {
        fprintf(stderr, "gettimeofday error\n");
        exit(1);
    }
    rearr_list();
    print_result();
    if (endtime.tv_usec < starttime.tv_usec) // when millisecond is negative
        printf("Searching time: %ld:%ld(sec:usec)\n\n", endtime.tv_sec - starttime.tv_sec - 1, 1000000 + endtime.tv_usec - starttime.tv_usec);
    else
        printf("Searching time: %ld:%ld(sec:usec)\n\n", endtime.tv_sec - starttime.tv_sec, endtime.tv_usec - starttime.tv_usec);
    if (start->next != NULL)
        command_sub();
    struct file_list *arrow;
    struct file_list *free_f;
    struct lower_list *arrow2;
    struct lower_list *free_l;
    arrow = start->next;
    while (arrow != NULL)
    {
        if (arrow->l_start != NULL)
        {
            arrow2 = arrow->l_start->next;
            while (arrow2 != NULL)
            {
                free_l = arrow2;
                arrow2 = arrow2->next;
                free(free_l);
            }
            free(arrow->l_start);
        }
        free_f = arrow;
        arrow = arrow->next;
        free(free_f);
    }
    free(start);
    exit(0);
}

void command_sub(void)
{
    int sub_argc = 0;
    char input[BUFMAX];
    char *sub_argv[SUB_ARGMAX];
    while (1)
    {
        printf(">> ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        sub_argc = split(input, " ", sub_argv);
        if (sub_argc == 0)
            continue;
        if (!strcmp(sub_argv[0], "exit"))
        {
            printf(">> Back to Prompt\n");
            break;
        }
        // else if (!strcmp(sub_argv[1], "d"))
        //     command_fmd5(sub_argc, sub_argv);
        // else if (!strcmp(sub_argv[1], "i"))
        //     command_fmd5(sub_argc, sub_argv);
        // else if (!strcmp(sub_argv[1], "f"))
        //     command_fmd5(sub_argc, sub_argv);
        // else if (!strcmp(sub_argv[1], "j"))
        //     command_fmd5(sub_argc, sub_argv);
        else
            printf("ERROR: Arguments error\n");
    }
}

int filt_dir(const struct dirent *dest)
{
	return((dest->d_type == DT_DIR)&&(strcmp(dest->d_name,"."))&&(strcmp(dest->d_name,"..")));
}

int filt_reg(const struct dirent *dest)
{
    if (!strlen(extension))
	    return((dest->d_type == DT_REG));
    else
        return((dest->d_type == DT_REG)&&(strstr(dest->d_name,extension) != NULL));
}

void search_src_file(char *dirname)
{
    struct dirent **dir_res;
    struct dirent **f_res;
    struct stat temp_stat;
    int dir_num;
	int f_num;
    long size = 0;
    char path[PATHMAX];
    char temp_path[PATHMAX];
    unsigned char md[MD5_DIGEST_LENGTH];

    f_num = scandir(dirname, &f_res, filt_reg, alphasort);
    dir_num = scandir(dirname, &dir_res, filt_dir, alphasort);
    if (f_num == -1 || dir_num == -1)
    {
        if (errno == 1)
			return;
		fprintf(stderr, "scandir error for %s\n", dirname);
		exit(1);
    }
    //dirname안에 있는 일반 파일 릭스트 <- f_res
    //dirname안에 있는 디렉토리 리스트 <- dir_res
    //먼저 해당 파일들 찾기.
	for (int i = 0; i < f_num; i++)
	{
        if (!strcmp(dirname, "/"))
			sprintf(path, "%s%s", dirname, f_res[i]->d_name);
		else
			sprintf(path, "%s/%s", dirname, f_res[i]->d_name);
        //path가 속성 확인하려는 파일임.
        if (lstat(path, &temp_stat) < 0)
        {
            fprintf(stderr, "lstat error for %s\n", path);
			exit(1);
        }//lstat으로 temp_stat에 path파일의 속성 가져옴.
        if (lminsize != -1 && lminsize > temp_stat.st_size)
            continue;
        if (lmaxsize != -1 && lmaxsize < temp_stat.st_size)
            continue;
        if (have_path(path))
            continue;
        //minsize나 maxsize가 명시되어 있는 경우에는 그 사이즈 안에 있는 파일들에 대해서만 탐색한다.
        do_fp(path, md);//md라는 unsigned char형 배열에 path파일에 해당하는 hash값을 넣어놓는다.
        strcpy(temp_path, target_dir);
        struct file_list *new_node = (struct file_list *)malloc(sizeof(struct file_list));
        create_new_node(new_node, md, path, temp_stat.st_size);
        search_file(temp_path, new_node);//같은 파일을 targetdir부터 찾는다.
        if (new_node->l_start == NULL)
            free(new_node);//같은 파일이 없으면 free시켜준다.
        else
            add_node(new_node);//같은 파일이 있으면 node를
    }
    for (int i = 0; i < dir_num; i++)
	{
        if (!strcmp(dirname, "/"))
			sprintf(path, "%s%s", dirname, dir_res[i]->d_name);
		else
			sprintf(path, "%s/%s", dirname, dir_res[i]->d_name);
        if (strcmp(path,"/run")&&strcmp(path,"/proc")&&strcmp(path,"/mnt/c")&&strcmp(path,"/sys"))
            search_src_file(path);
	}
	free(f_res);
	free(dir_res);
}

void search_file(char *dirname, struct file_list *node)
{
	struct dirent **dir_res;
    struct dirent **f_res;
    struct stat temp_stat;
    int dir_num;
	int f_num;
    long size = 0;
    char path[PATHMAX];
    unsigned char md[MD5_DIGEST_LENGTH];

    f_num = scandir(dirname, &f_res, filt_reg, alphasort);
    dir_num = scandir(dirname, &dir_res, filt_dir, alphasort);
    if (f_num == -1 || dir_num == -1)
    {
        if (errno == 1)
			return;
		fprintf(stderr, "scandir error for %s\n", dirname);
		exit(1);
    }
    //dirname안에 있는 일반 파일 릭스트 <- f_res
    //dirname안에 있는 디렉토리 리스트 <- dir_res
    //먼저 해당 파일들 찾기.
	for (int i = 0; i < f_num; i++)
	{
        if (!strcmp(dirname, "/"))
			sprintf(path, "%s%s", dirname, f_res[i]->d_name);
		else
			sprintf(path, "%s/%s", dirname, f_res[i]->d_name);
        //path가 속성 확인하려는 파일임.
        if (lstat(path, &temp_stat) < 0)
        {
            fprintf(stderr, "lstat error for %s\n", path);
			exit(1);
        }//lstat으로 temp_stat에 path파일의 속성 가져옴.
        if (lminsize != -1 && lminsize > temp_stat.st_size)
            continue;
        if (lmaxsize != -1 && lmaxsize < temp_stat.st_size)
            continue;
        //minsize나 maxsize가 명시되어 있는 경우에는 그 사이즈 안에 있는 파일들에 대해서만 탐색한다.
        if (have_path(path))
            continue;
        if (!strcmp(node->path, path))
            continue;
        do_fp(path, md);//md라는 unsigned char형 배열에 path파일에 해당하는 hash값을 넣어놓는다.
        if (node->size == temp_stat.st_size && hashcmp(node->md, md))
        {
            if (node->l_start == NULL)
            {
                node->l_start = (struct lower_list *)malloc(sizeof(struct lower_list));
                node->l_start->next = NULL;
            }
            add_low_node(node->l_start, path);
        }
	}
    for (int i = 0; i < dir_num; i++)
	{
        if (!strcmp(dirname, "/"))
			sprintf(path, "%s%s", dirname, dir_res[i]->d_name);
		else
			sprintf(path, "%s/%s", dirname, dir_res[i]->d_name);
        if (strcmp(path,"/run")&&strcmp(path,"/proc")&&strcmp(path,"/mnt/c")&&strcmp(path,"/sys"))
            search_file(path, node);
	}
	free(f_res);
	free(dir_res);
}

void add_node(struct file_list *new)
{
    struct file_list *temp;
    struct file_list *target;
    target = start;
    while (1)
    {
        if (target->next == NULL || target->next->size > new->size)
        {
            temp = target->next;
            target->next = new;
            new->next = temp;
            break;
        }
        target = target->next;
    }
}

void create_new_node(struct file_list *new, unsigned char *md, char *path, long size)
{
    strcpy(new->md, md);
    strcpy(new->path, path);
    new->size = size;
    new->l_start = NULL;
}

void add_low_node(struct lower_list *l_start, char *path)
{
    
    struct lower_list *new;
    struct lower_list *temp;
    struct lower_list *target;

    target = l_start;
    new = (struct lower_list *)malloc(sizeof(struct lower_list));
    while (target->next != NULL)
        target = target->next;
    temp = target->next;
    target->next = new;
    strcpy(new->path, path);
    new->next = temp;
}

void rearr_list(void)
{
    struct file_list *arrow;
    struct lower_list *temp;
    struct lower_list *fst_node;
    arrow = start->next;
    while (arrow != NULL)
    {
        temp = arrow->l_start->next;
        fst_node = (struct lower_list *)malloc(sizeof(struct lower_list));
        arrow->l_start->next = fst_node;
        fst_node->next = temp;
        strcpy(fst_node->path, arrow->path);
        arrow = arrow->next;
    }
}

bool have_path(char *path)
{
    struct file_list *arrow1;
    struct lower_list *arrow2;
    if (start->next == NULL)
        return false;
    arrow1 = start->next;
    while (1)
    {
        if (!strcmp(path, arrow1->path))
            return true;
        arrow2 = arrow1->l_start->next;
        while (1)
        {
            if (!strcmp(path, arrow2->path))
                return true;
            if (arrow2->next == NULL)
                break;
            arrow2 = arrow2->next;
        }
        if (arrow1->next == NULL)
            break;
        arrow1 = arrow1->next;
    }
    return false;
}

void pt(unsigned char *md)
{
    int i;

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x", md[i]);
}

void do_fp(char *path, unsigned char *md)
{
    MD5_CTX c;
    int fd;
    int i;
    unsigned char buf[BUFSIZE];
    FILE *f;

    if ((f = fopen(path, "r")) == NULL)
    {
        fprintf(stderr, "fopen error\n");
        exit(1);
    }
    fd = fileno(f);
    MD5_Init(&c);
    for (;;)
    {
        i = read(fd, buf, BUFSIZE);
        if (i <= 0)
            break;
        MD5_Update(&c, buf, (unsigned long)i);
    }
    MD5_Final(&(md[0]), &c);
    fclose(f);
}

bool hashcmp(unsigned char *dest, unsigned char *src)
{
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        if (dest[i] != src[i])
            return false;
    }
    return true;
}

void print_result(void)
{
    struct stat temp_stat;
    struct file_list *arrow;
    struct lower_list *arrow2;
    arrow = start->next;
    int i = 1;
    int j = 1;
    while (arrow != NULL)
    {
        if (arrow->l_start != NULL)
        {
            printf("---- Identical files #%d (%ld bytes - ", i, arrow->size);
            pt(arrow->md);//사이즈 0 3개마다 쉼표 나오게 할것!!!
            printf(") ----\n");
            i++;
            arrow2 = arrow->l_start->next;
            j = 1;
            while (arrow2 != NULL)
            {
                if (lstat(arrow2->path, &temp_stat) < 0)
                {
                    fprintf(stderr, "lstat error for %s\n", arrow2->path);
                    exit(1);
                }
                printf("[%d] %s (mtime", j, arrow2->path);
                print_time(temp_stat.st_mtim.tv_sec);
                printf(" (atime");
                print_time(temp_stat.st_atim.tv_sec);
                printf("\n");
                arrow2 = arrow2->next;
                j++;
            }
            printf("\n");
        }
        arrow = arrow->next;
    }
    if (start->next == NULL)
        printf("No duplicates in %s\n", target_dir);
}

void print_time(time_t tv_sec)
{
    printf(" : %2d-%02d-%02d %02d:%02d:%02d)",localtime(&tv_sec)->tm_year - 100, localtime(&tv_sec)->tm_mon + 1, localtime(&tv_sec)->tm_mday,
			   localtime(&tv_sec)->tm_hour, localtime(&tv_sec)->tm_min, localtime(&tv_sec)->tm_sec);
}