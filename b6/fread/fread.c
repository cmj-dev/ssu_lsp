#include <stdio.h>
#include <stdlib.h>

struct ssu_pirate 
{
    unsigned long booty;
    unsigned int bread_length;
    char name[128];
};

int main(void)
{
    struct ssu_pirate blackbeard = {950, 48, "Edward Teach"}, pirate;
    char *fname = "ssu_data";
    FILE *fp1, *fp2;

    if ((fp2 = fopen(fname, "w")) == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", fname);
        exit(1);
    }

    
}