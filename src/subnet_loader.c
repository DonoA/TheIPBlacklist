#include "subnet_loader.h"

#include "blacklist.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

subnet_t parse_subnet(char *str)
{
    subnet_t rtn;
    rtn.address = 0;
    rtn.sig_bits = 32;

    char *curr = str;
    char *buf_start = str;
    // take in 4 bytes
    for (size_t i = 0; i < 4; i++)
    {
        while (true)
        {
            if ('0' <= *curr && *curr <= '9')
            {
                curr++;
                continue;
            }
            char tmp = *curr;
            *curr = 0;
            rtn.address += atoi(buf_start) << ((3 - i) * 8);
            *curr = tmp;
            curr++;
            buf_start = curr;
            break;
        }
    }

    // take in significant bit mask
    if (*curr != 0)
    {
        rtn.sig_bits = atoi(curr);
    }

    assert(rtn.sig_bits <= 32);

    return rtn;
}

void strip(char *str)
{
    char *last = str + strlen(str) - 1;
    if (*last == '\n')
    {
        *last = 0;
    }
}

bool isDir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

void parseFile(vector_t *subnet_vector, char *infile);
void parseFolder(vector_t *subnet_vector, char *infile);

void parseFolder(vector_t *subnet_vector, char *infile)
{
    DIR *d = opendir(infile);
    struct dirent *dir;
    if (!d)
    {
        return;
    }

    while ((dir = readdir(d)) != NULL)
    {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
        {
            continue;
        }

        char path[1024] = {0};
        strcat(path, infile);
        strcat(path, "/");
        strcat(path, dir->d_name);
        parseFile(subnet_vector, path);
    }

    closedir(d);
}

void parseFile(vector_t *subnet_vector, char *infile)
{
    if (isDir(infile))
    {
        parseFolder(subnet_vector, infile);
        return;
    }

    FILE *fp = fopen(infile, "r");
    if (fp == NULL)
    {
        printf("Filed to open file %s\n", infile);
        perror("File open error");
        exit(EXIT_FAILURE);
    }

    ssize_t read = 0;
    size_t len = 0;
    char *line = NULL;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (read == 1)
        {
            continue;
        }
        strip(line);
        if (line[0] == '#')
        {
            continue;
        }
        subnet_t subnet = parse_subnet(line);
        vectorAdd(subnet_vector, &subnet);
    }

    fclose(fp);
    free(line);
}