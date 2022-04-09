#include "ssu_find.h"

void command_sub(int list_leng)
{
    int set_index;
    int list_idx;
    int sub_argc = 0;
    char input[BUFMAX];
    char *sub_argv[SUB_ARGMAX];
    while (1)
    {
        if (list_leng == 0)
            break;
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
        else if (is_digit(sub_argv[0]) && sub_argc > 1 && (set_index = atoi(sub_argv[0])) > 0 && set_index <= list_leng)
        {
            if (!strcmp(sub_argv[1], "d") && sub_argc == 3 && is_digit(sub_argv[2]))
            {
                list_idx = atoi(sub_argv[2]);
                option_command(set_index, list_idx, 1);
                // printf("\"%s\" has been deleted in #%d\n", get_node_by_index(set_index, list_idx, 'n'), set_index);
                printf("\n");
                list_leng = print_result();
            }
            else if (!strcmp(sub_argv[1], "i"))
            {
                option_command(set_index, list_idx, 2);
                printf("\n");
                list_leng = print_result();
            }
            else if (!strcmp(sub_argv[1], "f"))
            {
                option_command(set_index, list_idx, 3);
                printf("\n");
                list_leng = print_result();
            }
            else if (!strcmp(sub_argv[1], "t"))
            {
                option_command(set_index, list_idx, 4);
                printf("\n");
                list_leng = print_result();
            }
            else
                printf("ERROR: Arguments error\n");
        }
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

void delete_list(void)
{
    struct file_list *arrow_i;
    struct file_list *arrow_b;
    struct lower_list *arrow_j;
    int i;
    int j;

    if (start->next == NULL)
        return;
    arrow_b = start;
    arrow_i = start->next;
    while (1)
    {
        arrow_j = arrow_i->l_start;
        if (arrow_j->next == NULL)
        {
            free(arrow_j);
            arrow_i = arrow_i->next;
            free(arrow_b->next);
            arrow_b->next = arrow_i;
        }
        else if (arrow_j->next->next == NULL)
        {
            free(arrow_j->next);
            free(arrow_j);
            arrow_i = arrow_i->next;
            free(arrow_b->next);
            arrow_b->next = arrow_i;
        }
        else
        {
            arrow_b = arrow_i;
            arrow_i = arrow_i->next;
        }
        if (arrow_i == NULL)
            break;
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

bool hashcmp(unsigned char *dest, unsigned char *src)
{
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        if (dest[i] != src[i])
            return false;
    }
    return true;
}

void print_time(time_t tv_sec)
{
    printf("20%2d-%02d-%02d %02d:%02d:%02d",localtime(&tv_sec)->tm_year - 100, localtime(&tv_sec)->tm_mon + 1, localtime(&tv_sec)->tm_mday,
			   localtime(&tv_sec)->tm_hour, localtime(&tv_sec)->tm_min, localtime(&tv_sec)->tm_sec);
}

void print_num(long size)
{
    if (size < 1000)
    {
        printf("%ld",size);
    }
    else
    {
        print_num(size / 1000);
        printf(",%03ld",size % 1000);
    }
}

bool is_digit(char *num)
{
    for (int i = 0; num[i] != '\0'; i++)
    {
        if (num[i] < '0' || num[i] > '9' )
            return false;
    }
    return true;
}

void option_command(int set_index, int list_idx, int option)
{
    struct file_list *arrow_i;
    struct lower_list *arrow_j;
    struct lower_list *arrow_b;
    struct lower_list *temp;
    struct stat temp_stat;
    time_t m_time;

    m_time = 0;
    arrow_i = start;
    for (int i = 1; i <= set_index; i++)
        arrow_i = arrow_i->next;
    arrow_j = arrow_i->l_start;
    if (option == 1)
    {
        for (int j = 1; j <= list_idx; j++)
        {
            if (arrow_j->next == NULL)
            {
                printf("list_idx is out of range\n");
                return;
            }
            arrow_b = arrow_j;
            arrow_j = arrow_j->next;
        }
        if (unlink(arrow_j->path) < 0)
            fprintf(stderr, "unlink error %s", arrow_j->path);
        printf("\"%s\" has been deleted in #%d\n", arrow_j->path, set_index);
        arrow_b->next = arrow_j->next;
        free(arrow_j);
    }
    else if (option == 2)
    {
        char buf[BUFMAX];
        for ( ; ; )
        {
            if (arrow_j->next == NULL)
                break;
            arrow_b = arrow_j;
            arrow_j = arrow_j->next;
            printf("Delete \"%s\"? ", arrow_j->path);
            fgets(buf, sizeof(buf), stdin);
            buf[strlen(buf) - 1] = '\0';
            if (strlen(buf) == 1 && (buf[0] == 'y' || buf[0] == 'Y'))
            {
                if (unlink(arrow_j->path) < 0)
                    fprintf(stderr, "unlink error %s", arrow_j->path);
                arrow_b->next = arrow_j->next;
                free(arrow_j);
                arrow_j = arrow_b;
            }
            else if (strlen(buf) == 1 && (buf[0] == 'n' || buf[0] == 'N'))
            {
            }
            else
            {
                printf("ERROR: Arguments error\n");
                return;
            }
        }
    }
    else if (option == 3)
    {
        for (;;)
        {
            if (arrow_j->next == NULL)
                break;
            arrow_j = arrow_j->next;
            if (lstat(arrow_j->path, &temp_stat) < 0)
            {
                fprintf(stderr, "lstat error for %s\n", arrow_j->path);
                exit(1);
            }
            if (m_time < temp_stat.st_mtim.tv_sec)
            {
                m_time = temp_stat.st_mtim.tv_sec;
                temp = arrow_j;
            }
        }
        arrow_j = arrow_i->l_start;
        for (;;)
        {
            if (arrow_j->next == NULL)
                break;
            arrow_b = arrow_j;
            arrow_j = arrow_j->next;
            if (arrow_j != temp)
            {
                if (unlink(arrow_j->path) < 0)
                    fprintf(stderr, "unlink error %s", arrow_j->path);
                arrow_b->next = arrow_j->next;
                free(arrow_j);
                arrow_j = arrow_b;
            }
        }
        printf("Left file in #%d : %s (", set_index, temp->path);
        print_time(m_time);
        printf(")\n");
    }
    else if (option == 4)
    {
        for (;;)
        {
            if (arrow_j->next == NULL)
                break;
            arrow_j = arrow_j->next;
            if (lstat(arrow_j->path, &temp_stat) < 0)
            {
                fprintf(stderr, "lstat error for %s\n", arrow_j->path);
                exit(1);
            }
            if (m_time < temp_stat.st_mtim.tv_sec)
            {
                m_time = temp_stat.st_mtim.tv_sec;
                temp = arrow_j;
            }
        }
        arrow_j = arrow_i->l_start;
        for (;;)
        {
            if (arrow_j->next == NULL)
                break;
            arrow_b = arrow_j;
            arrow_j = arrow_j->next;
            if (arrow_j != temp)
            {
                trash(arrow_j->path);
                arrow_b->next = arrow_j->next;
                free(arrow_j);
                arrow_j = arrow_b;
            }
        }
        printf("Left file in #%d : %s (", set_index, temp->path);
        print_time(m_time);
        printf(")\n");
    }
}

void trash(char *path)
{
    char file[FILEMAX];
    int tail_num;
    char tail[FILEMAX] = "00000000000000000000";
    char trash_path[PATHMAX];
    char trash_info_path[PATHMAX];
    int f_num;
    struct dirent **f_res;
    struct timeval deltime;
    FILE *fp;

    tail_num = 1;
    strcpy(trash_path, getenv("HOME"));
    strcpy(trash_info_path, getenv("HOME"));
    strcat(trash_path, "/.local/share/Trash/files");
    strcat(trash_info_path, "/.local/share/Trash/info/");
    strcpy(file, strrchr(path, '/') + 1);
    f_num = scandir(trash_path, &f_res, NULL, alphasort);
    if (f_num == -1)
    {
        if (errno == 1)
            return;
        fprintf(stderr, "scandir error for %s\n", trash_path);
        exit(1);
    }
    for (int i = 0; i < f_num; i++)
    {
        if (!strcmp(f_res[i]->d_name, file))
        {
            strcpy(file, strrchr(path, '/') + 1);
            itoa(tail_num, tail);
            strcat(file, strrchr(tail, '_'));
            tail_num++;
            i = 0;
        }
    }
    strcat(trash_path, "/");
    strcat(trash_path, file);
    rename(path, trash_path);
    strcat(file,".trashinfo");
    fp = fopen(strcat(trash_info_path, file), "w");
    if (gettimeofday(&deltime, NULL) < 0) // get deltime
    {
        fprintf(stderr, "gettimeofday error\n");
        exit(1);
    }
    // printf("%s\n", trash_path);
    // printf("%s\n", trash_info_path);
    fprintf(fp, "[Trash Info]\nPath=%s\nDeletionDate=20%2d-%02d-%02dT%02d:%02d:%02d\n", path,
            localtime(&deltime.tv_sec)->tm_year - 100, localtime(&deltime.tv_sec)->tm_mon + 1, localtime(&deltime.tv_sec)->tm_mday,
            localtime(&deltime.tv_sec)->tm_hour, localtime(&deltime.tv_sec)->tm_min, localtime(&deltime.tv_sec)->tm_sec);
    // printf("[Trash Info]\n\rPath=%s\n\rDeletionDate=20%2d-%02d-%02dT%02d:%02d:%02d\n", path,
    //         localtime(&deltime.tv_sec)->tm_year - 100, localtime(&deltime.tv_sec)->tm_mon + 1, localtime(&deltime.tv_sec)->tm_mday,
    //         localtime(&deltime.tv_sec)->tm_hour, localtime(&deltime.tv_sec)->tm_min, localtime(&deltime.tv_sec)->tm_sec);
    fclose(fp);
}

void itoa(int num, char *c)
{
    int i = 15;
    c[16] = '\0';
    while (num > 10)
    {
        c[i] = '0' + num % 10;
        num /= 10;
        i--;
    }
    c[i] = '0' + num % 10;
    i--;
    c[i] = '_';
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