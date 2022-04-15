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
        if (list_leng == 0)//더 이상 중복리스트가 없는 경우 종료.
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
{//현재 디렉토리와 부모 디렉토리를 제외하고 검색
	return((dest->d_type == DT_DIR)&&(strcmp(dest->d_name,"."))&&(strcmp(dest->d_name,"..")));
}

int filt_reg(const struct dirent *dest)
{//확장자가 정해지지 않은 경우에는 정규파일 탐색 확장자가 정해진 경우에는 이름이 해당 확장자로 끝나는 정규파일만 검색.
    if (!strlen(extension))
	    return((dest->d_type == DT_REG));
    else
        return((dest->d_type == DT_REG)&&(strstr(dest->d_name,extension) != NULL)&&(strstr(dest->d_name,extension)+strlen(extension))[0] == '\0');
}

void add_node(struct file_list *new)
{//size순으로 중복 링크드리스트의 헤더 노드 추가.
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
{//새로운 중복 링크드리스트 헤더 노드 생성.
    strcpy(new->md, md);
    strcpy(new->path, path);
    new->size = size;
    new->l_start = NULL;
}

void add_low_node(struct lower_list *l_start, char *path)
{
    //중복 링크드리스트에 맴버 추가. BFS이므로 자동으로 깊이 순으로 추가됨.
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
{//중복 링크드리스트의 헤더에 있는 기준이 되었던 파일 정보를 링크드 리스트의 맨 처음에 추가.
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
{//중복 링크드리스트의 헤더 링크드리스트 중 맴버가 한개이거나 없는 헤더 정리.
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
        {//맴버가 없는 경우
            free(arrow_j);
            arrow_i = arrow_i->next;
            free(arrow_b->next);
            arrow_b->next = arrow_i;
        }
        else if (arrow_j->next->next == NULL)
        {//맴버가 한개인 경우
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
{//링크드 리스트 검색하면서 해당 path와 같은 path가 이미 저장되어 있는지 확인.
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
{//매개변수로 받은 두 해시값 비교. 같으면 true 아니면 false return.
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        if (dest[i] != src[i])
            return false;
    }
    return true;
}

void print_time(time_t tv_sec)
{//time_t구조체를 받아서 YY-MM-DD HH:MM:SS 형식으로 출력.
    printf("20%2d-%02d-%02d %02d:%02d:%02d",localtime(&tv_sec)->tm_year - 100, localtime(&tv_sec)->tm_mon + 1, localtime(&tv_sec)->tm_mday,
			   localtime(&tv_sec)->tm_hour, localtime(&tv_sec)->tm_min, localtime(&tv_sec)->tm_sec);
}

void print_num(long size)
{//세자리수마다 , 추가하여 숫자 출력.
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
{//배열이 숫자로만 이루어져 있는지 확인.
    for (int i = 0; num[i] != '\0'; i++)
    {
        if (num[i] < '0' || num[i] > '9' )
            return false;
    }
    return true;
}

void option_command(int set_index, int list_idx, int option)
{// >>커멘드의 옵션 처리.
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
    if (option == 1)// 옵션 d
    {
        for (int j = 1; j <= list_idx; j++)
        {//사용자가 입력한 인덱스 까지 이동.
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
        free(arrow_j);//해당 인덱스 파일 삭제 후 링크드 리스트에서도 삭제
    }
    else if (option == 2)//옵션 i
    {
        char buf[BUFMAX];
        for ( ; ; )
        {
            if (arrow_j->next == NULL)
                break;
            arrow_b = arrow_j;
            arrow_j = arrow_j->next;
            printf("Delete \"%s\"? [y/n] ", arrow_j->path);
            fgets(buf, sizeof(buf), stdin);
            buf[strlen(buf) - 1] = '\0';
            if (strlen(buf) == 1 && (buf[0] == 'y' || buf[0] == 'Y'))
            {//y나 Y가 입력될 경우에는 해당 인덱스 파일 삭제 후 링크드 리스트에서도 삭제.
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
    else if (option == 3)//옵션 f
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
        }//temp에 mtime이 가장 최근인 파일의 노드 저장. 만약 mtime이 같다면 더 작은 인덱스의 파일이 생존.
        arrow_j = arrow_i->l_start;
        for (;;)
        {
            if (arrow_j->next == NULL)
                break;
            arrow_b = arrow_j;
            arrow_j = arrow_j->next;
            if (arrow_j != temp)
            {//temp에 들어있는 파일을 제외한 모든 파일 삭제.
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
    else if (option == 4)//옵션 t
    {//f와 로직은 같고 unlink를 trash로 변경.
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
                trash(arrow_j->path);//휴지통으로 해당 파일 전송.
                arrow_b->next = arrow_j->next;
                free(arrow_j);
                arrow_j = arrow_b;
            }
        }
        printf("All files in #%d have moved to Trash except %s (", set_index, temp->path);
        print_time(m_time);
        printf(")\n");
    }
}

void trash(char *path)
{
    char file[FILEMAX];
    int tail_num;
    char tail[FILEMAX] = "00000000000000000000";
    char trash_path[PATHMAX];//휴지통의 위치.
    char trash_info_path[PATHMAX];//휴지통에 들어간 파일의 정보파일이 들어갈 위치.
    int f_num;
    struct dirent **f_res;
    struct timeval deltime;
    FILE *fp;

    tail_num = 1;
    strcpy(trash_path, getenv("HOME"));
    strcpy(trash_info_path, getenv("HOME"));
    strcat(trash_path, "/.local/share/Trash/files");
    strcat(trash_info_path, "/.local/share/Trash/info/");
    strcpy(file, strrchr(path, '/') + 1);//파일의 이름을 path에서 추출
    f_num = scandir(trash_path, &f_res, NULL, alphasort);// 휴지통에서 파일 이름 중복 방지 위해 휴지통 탐색
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
        {//이름이 중복되는 파일이 있는 경우 휴지통으로 보낼 파일의 이름 뒤에 _n을 1부터 붙여줌.
            strcpy(file, strrchr(path, '/') + 1);
            itoa(tail_num, tail);//tailnum을 문자열로 변환
            strcat(file, strrchr(tail, '_'));
            tail_num++;
            i = 0;//다시 파일을 처음부터 n이 tailnum일때 겹치는 파일이 있는지 검색.
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
    fprintf(fp, "[Trash Info]\nPath=%s\nDeletionDate=20%2d-%02d-%02dT%02d:%02d:%02d\n", path,
            localtime(&deltime.tv_sec)->tm_year - 100, localtime(&deltime.tv_sec)->tm_mon + 1, localtime(&deltime.tv_sec)->tm_mday,
            localtime(&deltime.tv_sec)->tm_hour, localtime(&deltime.tv_sec)->tm_min, localtime(&deltime.tv_sec)->tm_sec);// 추후 trash-restore를 위한 trashinfo파일 생성 후 저장.
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
    }//뒤에서부터 10으로 나눈 수를 char로 변환 후 저장.
    c[i] = '0' + num % 10;
    i--;
    c[i] = '_';//맨 앞에는 _추가.
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