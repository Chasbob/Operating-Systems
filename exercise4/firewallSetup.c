// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 80
int main(int argc, char *argv[])
{
    printf("test");
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char buffer[BUFFERSIZE];
    printf("%s\n",argv[2]);
    if (argc == 2 && strcmp(argv[1], "L"))
    {
        printf("list rules\n");
    }
    else if (argc == 3)
    {
        printf("argv[2] = %s\n", argv[2]);
        fp = fopen(argv[2], "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);
        while ((read = getline(&line, &len, fp)) != -1)
        {
            strtok(line,"\n");
            snprintf(buffer, BUFFERSIZE, "echo -n \"A %.*s\" > /proc/firewallExtension",(int)len-2, line);
            printf("%s\n", buffer);
            system(buffer);
        }
        fclose(fp);
        if (line)
            free(line);
        exit(EXIT_SUCCESS);
    }
}