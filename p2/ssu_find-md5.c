#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/md5.h>

#define ARGMAX 5
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
    struct lower_list *down;
};

struct file_list *start;
long lminsize;
long lmaxsize;

unsigned char *do_fp(char *path);
void search_file(char *dirname);
void pt(unsigned char *md);
void add_node(struct file_list *target, unsigned char *md, char *path, long size);
void add_low_node(struct lower_list *target, char *path);

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
    {
        printf("for all file\n");
    }
    else if (!strncmp(argv[1], "*.", 2))
    {
        printf("find for extension %s\n", &argv[1][1]);
    }
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
        fprintf(stderr, "realpath error \n");
        exit(1);
    }
    lminsize = (long)minsize;
    lmaxsize = (long)maxsize;
    start = (struct file_list*)malloc(sizeof(struct file_list));
    search_file(rpath);
    struct file_list *arrow;
    arrow = start;
    printf("%ld, %ld, %s\n", lminsize, lmaxsize, rpath);
    while (arrow != NULL)
    {
        printf("path : %s\t", arrow->path);
        printf("size : %ld\t", arrow->size);
        pt(arrow->md);
        printf("\n");
        arrow = arrow->next;
    }
    
}

int filt_dir(const struct dirent *dest)
{
	return((dest->d_type == DT_DIR)&&(strcmp(dest->d_name,"."))&&(strcmp(dest->d_name,"..")));
}

int filt_reg(const struct dirent *dest)
{
	return((dest->d_type == DT_REG));
}

void search_file(char *dirname)
{
	struct dirent **dir_res;
    struct dirent **f_res;
    struct file_list *arrow;
    struct lower_list *low_arrow;
    struct file_list new_filelist;
    struct stat temp_stat;
    int dir_num;
	int f_num;
    long size = 0;
    char path[PATHMAX];
    unsigned char md[MD5_DIGEST_LENGTH];

    f_num = scandir(dirname, &f_res, filt_reg, alphasort);
    dir_num = scandir(dirname, &dir_res, filt_dir, alphasort);
    printf("%d %d\n",f_num, dir_num);
    if (f_num == -1 || dir_num == -1)
    {
        if (errno == 1)
			return;
		fprintf(stderr, "scandir error for %s\n", dirname);
		exit(1);
    }
    printf("start dir name %s\n",dirname);
	for (int i = 0; i < f_num; i++)
	{
        if (!strcmp(dirname, "/"))
			sprintf(path, "%s%s", dirname, f_res[i]->d_name);
		else
			sprintf(path, "%s/%s", dirname, f_res[i]->d_name);
        printf("file to search %s\n",path);
        if (lstat(path, &temp_stat) < 0)
        {
            fprintf(stderr, "lstat error for %s\n", path);
			exit(1);
        }
        if (lminsize != -1 && lminsize > temp_stat.st_size)
            continue;
        if (lmaxsize != -1 && lmaxsize < temp_stat.st_size)
            continue;
        strcpy(md, do_fp(path));
        printf("file to search %s\n",path);
		if (start == NULL)
        {
            add_node(start, md, path, temp_stat.st_size);
            printf("path : %s\t", start->next->path);
            printf("size : %ld\t", start->next->size);
            printf("next pointer : %p\t", start->next->next);
            pt(start->next->md);
            printf("\n");
        }
        else
        {
            arrow = start->next;
            while (arrow != NULL)
            {
                if (arrow->size == temp_stat.st_size)
                {
                    if (!strcmp(arrow->md, md))
                    {
                        struct lower_list *new_llist;
                        if (arrow->down == NULL)
                        {
                            strcpy(new_llist.path, path);
                            new_llist.next = NULL;
                            arrow->down = &new_llist;
                        }
                        else
                        {
                            low_arrow = arrow->down;
                            while (low_arrow->next != NULL)
                                low_arrow = low_arrow->next;
                            strcpy(new_llist.path, path);
                            new_llist.next = NULL;
                            low_arrow->next = &new_llist;
                        }
                        break;
                    }
                }
                printf("path : %s\t", arrow->path);
                printf("size : %ld\t", arrow->size);
                printf("current pointer : %p\t", arrow);
                printf("next pointer : %p\t", arrow->next);
                pt(arrow->md);
                printf("\n");
                arrow = arrow->next;
                
            }
            if (arrow->next == arrow)
            {
                strcpy(new_filelist.md, md);
                strcpy(new_filelist.path, path);
                new_filelist.size = temp_stat.st_size;
                new_filelist.down = NULL;
                new_filelist.next = NULL;
                arrow->next = &new_filelist;
                printf("path : %s\t", new_filelist.path);
                printf("size : %ld\t", new_filelist.size);
                printf("current pointer : %p\t", &new_filelist);
                printf("next pointer : %p\t", new_filelist.next);
                pt(new_filelist.md);
                printf("\n");
            }
        }
	}
	dir_num = scandir(dirname, &dir_res, filt_dir, alphasort);
    for (int i = 0; i < dir_num; i++)
	{
        if (!strcmp(dirname, "/"))
			sprintf(path, "%s%s", dirname, f_res[i]->d_name);
		else
			sprintf(path, "%s/%s", dirname, f_res[i]->d_name);
        search_file(path);
	}
	free(f_res);
	free(dir_res);
}

void add_node(struct file_list *target, unsigned char *md, char *path, long size)
{
    struct file_list *new;
    struct file_list *temp;
    new = (struct file_list *)malloc(sizeof(struct file_list));
    temp = target->next;
    target->next = new;
    strcpy(new->md, md);
    strcpy(new->path, path);
    new->size = size;
    new->next = temp;
    new->down = NULL;
}

void add_low_node(struct lower_list *target, char *path)
{
    struct lower_list *new;
    struct lower_list *temp;
    new = (struct lower_list *)malloc(sizeof(struct lower_list));
    temp = target->next;
    target->next = new;
    strcpy(new->path, path);
    new->next = temp;
}

void pt(unsigned char *md)
{
    int i;

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x", md[i]);
}

unsigned char *do_fp(char *path)
{
    MD5_CTX c;
    static unsigned char md[MD5_DIGEST_LENGTH];
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
    return md;
}