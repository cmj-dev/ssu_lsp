#ifndef __SSU_FIND_H__

# define __SSU_FIND_H__
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
#include <openssl/sha.h>
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
    unsigned char md[SHA_DIGEST_LENGTH];
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

int filt_dir(const struct dirent *dest);
int filt_reg(const struct dirent *dest);
void command_sub(int list_leng);
void search_src_file(char *dirname);
void search_file(char *dirname, struct file_list *node);
void add_node(struct file_list *);
void create_new_node(struct file_list *, unsigned char *, char *, long);
void add_low_node(struct lower_list *l_start, char *path);
bool hashcmp(unsigned char *dest, unsigned char *src);
void filter_list(void);
bool have_path(char *path);
int print_result(void);
void rearr_list(void);
void print_time(time_t tv_sec);
void print_num(long size);
bool is_digit(char *num);
void option_command(int set_index, int list_idx, int option);
void trash(char *path);
void itoa(int num, char *c);
int split(char* string, char* seperator, char* argv[]);
void delete_list(void);

#endif