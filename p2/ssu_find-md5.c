#include "ssu_find.h"

void pt(unsigned char *md);
void do_fp(char *path, unsigned char *md);

int main(int argc, char **argv)
{
    double minsize;
    double maxsize;
    char rpath[PATHMAX];
    char path[PATHMAX];
    int list_leng;
    if (argc != ARGMAX)
    {
        printf("ERROR: Arguments error\n");
        exit(1);
    }
    if (!strcmp(argv[1], "*"))
        strcpy(extension, "");
    else if (!strncmp(argv[1], "*.", 2))
        strcpy(extension, &argv[1][1]);
    else
    {
        printf("please enter the correct extension\n");
        exit(1);
    }//확장자 처리

    if (strcmp(argv[2],"~"))
    {
        minsize = atof(argv[2]);
        if (minsize < 1)
        {
            printf("please enter the correct size\n");
            exit(1);
        }
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
        if (maxsize < 1)
        {
            printf("please enter the correct size\n");
            exit(1);
        }
        if (strstr(argv[3],"MB") != NULL || strstr(argv[3],"mb") != NULL)
            maxsize *= 1000000;
        else if (strstr(argv[3],"GB") != NULL || strstr(argv[3],"gb") != NULL)
            maxsize *= 1000000000;
        else if (strstr(argv[3],"KB") != NULL || strstr(argv[3], "kb") != NULL)
            maxsize *= 1000;
    }
    else
        maxsize = -1;
    //min, max사이즈 단위 byte로 변경, 입력값이 ~면 고려하지 않고 탐색 진행
    if (!strncmp(argv[4], "~", 1))
    {
        strcpy(path, getenv("HOME"));
        strcat(path, strstr(argv[4], "~") + 1);
    }
    else
        strcpy(path, argv[4]);
    // ~인 경우에는 사용자의 홈디렉토리 환경변수 받아와서 진행
    if (realpath(path, rpath) == NULL)
    {
        fprintf(stderr, "targetpass error \n");
        exit(1);
    }//targetpass 가 없을 경우 error 출력
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
    search_src_file(rpath);//탐색 진행
    if (gettimeofday(&endtime, NULL) < 0) // get endtime
    {
        fprintf(stderr, "gettimeofday error\n");
        exit(1);
    }
    rearr_list();//프린트하기 알맞게 링크드리스트 재배치
    list_leng = print_result();//중복파일 리스트의 개수 리턴
    if (list_leng == 0)
        printf("No duplicates in %s\n",target_dir);
    if (endtime.tv_usec < starttime.tv_usec)
        printf("Searching time: %ld:%ld(sec:usec)\n\n", endtime.tv_sec - starttime.tv_sec - 1, 1000000 + endtime.tv_usec - starttime.tv_usec);
    else
        printf("Searching time: %ld:%ld(sec:usec)\n\n", endtime.tv_sec - starttime.tv_sec, endtime.tv_usec - starttime.tv_usec);
    if (start->next != NULL)
        command_sub(list_leng);// >>명령어 진행
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
    exit(0); //free 진행 후 종료
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
    //dirname안에 있는 일반 파일 리스트 <- f_res
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
        if (temp_stat.st_size < 1)
            continue;
        if (have_path(path))
            continue;
        //minsize나 maxsize가 명시되어 있는 경우에는 그 사이즈 안에 있는 파일들에 대해서만 탐색한다.
        //사이즈가 0이거나 path가 이미 저장된 링크드 리스트 안에 있는 파일이면 진행하지 않는다.
        do_fp(path, md);//md라는 unsigned char형 배열에 path파일에 해당하는 hash값을 넣어놓는다.
        strcpy(temp_path, target_dir);
        struct file_list *new_node = (struct file_list *)malloc(sizeof(struct file_list));
        create_new_node(new_node, md, path, temp_stat.st_size);//new_node에 비교 기준 파일의 정보를 넣어놓는다.
        search_file(temp_path, new_node);//같은 파일을 targetdir부터 찾는다.
        if (new_node->l_start == NULL)
            free(new_node);//같은 파일이 없으면 free시켜준다.
        else
            add_node(new_node);//같은 파일이 있으면 node를 리스트에 추가한다.
    }
    for (int i = 0; i < dir_num; i++)
	{
        if (!strcmp(dirname, "/"))
			sprintf(path, "%s%s", dirname, dir_res[i]->d_name);
		else
			sprintf(path, "%s/%s", dirname, dir_res[i]->d_name);
        if (strcmp(path,"/run")&&strcmp(path,"/proc")&&strcmp(path,"/mnt/c")&&strcmp(path,"/sys"))
            search_src_file(path);//BFS로 진행한다.
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
    //dirname안에 있는 일반 파일 리스트 <- f_res
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
        if (!strcmp(node->path, path))//기준파일과 같은 파일이면 무시
            continue;
        if (node->size != temp_stat.st_size)//사이즈가 다르면 무시
            continue;
        do_fp(path, md);//md라는 unsigned char형 배열에 path파일에 해당하는 hash값을 넣어놓는다.
        if (hashcmp(node->md, md))//기준 파일과 hash값을 비교해서 같으면 넣기를 진행.
        {
            if (node->l_start == NULL)
            {//노드의 첫번째가 없으면 malloc으로 할당 후 추가
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
            search_file(path, node);//BFS로 비교 진행
	}
	free(f_res);
	free(dir_res);
}

void pt(unsigned char *md)
{
    int i;

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x", md[i]);//md5 한칸씩 프린트
}

void do_fp(char *path, unsigned char *md)
{//해시값 md에 저장
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

int print_result(void)
{//링크드 리스트에 저장된 대로 출력
    delete_list();
    struct stat temp_stat;
    struct file_list *arrow;
    struct lower_list *arrow2;
    arrow = start->next;
    int i = 0;
    int j = 1;
    while (arrow != NULL)
    {
        if (arrow->l_start != NULL)
        {
            i++;
            printf("---- Identical files #%d (", i);
            print_num(arrow->size);
            printf(" bytes - ");
            pt(arrow->md);
            printf(") ----\n");
            arrow2 = arrow->l_start->next;
            j = 1;
            while (arrow2 != NULL)
            {
                if (lstat(arrow2->path, &temp_stat) < 0)
                {
                    fprintf(stderr, "lstat error for %s\n", arrow2->path);
                    exit(1);
                }
                printf("[%d] %s (mtime : ", j, arrow2->path);
                print_time(temp_stat.st_mtim.tv_sec);
                printf(") (atime : ");
                print_time(temp_stat.st_atim.tv_sec);
                printf(")\n");
                arrow2 = arrow2->next;
                j++;
            }
            printf("\n");
        }
        arrow = arrow->next;
    }
    return i;//총 프린트한 중복리스트 개수 반환
}
