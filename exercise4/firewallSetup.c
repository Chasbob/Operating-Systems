// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 80
int main(int argc, char *argv[])
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char buffer[BUFFERSIZE];
    if (argc == 2 && strcmp(argv[1], "L"))
    {
        snprintf(buffer, BUFFERSIZE, "echo -n \"L\" > /proc/firewallExtension");
        system(buffer);
    }
    if (argc == 3)
    {
        snprintf(buffer, BUFFERSIZE, "echo -n \"R\" > /proc/firewallExtension");
        system(buffer);
        fp = fopen(argv[2], "r");
        if (fp == NULL)
            exit(EXIT_FAILURE);
        while ((read = getline(&line, &len, fp)) != -1)
        {
            strtok(line,"\n");
            snprintf(buffer, BUFFERSIZE, "echo -n \"A %.*s\" > /proc/firewallExtension",(int)len-2, line);
            system(buffer);
        }
        fclose(fp);
        if (line)
            free(line);
        exit(EXIT_SUCCESS);
    }
}