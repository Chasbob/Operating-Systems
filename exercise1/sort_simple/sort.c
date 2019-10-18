#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 20
#define MAX_LINES 20


char **parseFile(FILE *fp, char *lines[], int *count) {
    char line[MAX_LENGTH];
    while (fgets(line, sizeof(line), fp)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        int l = strlen(line) - 1;
        if (strcmp(&line[l], "\n") == 0) {

            lines[*count] = malloc(sizeof(line));
        } else {
            lines[*count] = malloc(sizeof(line) + 1);
            strcat(line, "\n");
        }
        strcpy(lines[*count], line);
        (*count)++;
    }
    return lines;
}

void swap(char *xp, char *yp) {
    char *temp = malloc(sizeof(xp));
    strcpy(temp, xp);
    strcpy(xp, yp);
    strcpy(yp, temp);
    free(temp);
}

void bubbleSort(char *arr[], int n) {
    int i, j;
    for (i = 0; i < n - 1; i++)

        // Last i elements are already in place
        for (j = 0; j < n - i - 1; j++)
            if (strcmp(arr[j], arr[j + 1]) > 0) {
                swap(arr[j], arr[j + 1]);
            }
}

int main(int argc, char **argv) {
    FILE *fp;

    if (argc > 1) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            fprintf(stderr, "cannot open %s\n", argv[1]);
            return 1;
        }
    } else {
        fp = stdin; /* read from standard input if no argument on the command line */
    }
    char *unsorted[MAX_LINES];
    int *size = malloc(sizeof(int*));
    *size = 0;
    parseFile(fp, unsorted, size);
    bubbleSort(unsorted, *size);
    for (int i = 0; i < *size; ++i) {
        printf("%s", unsorted[i]);
    }
    for (size_t i = 0; i < *size; i++)
    {
        free(unsorted[i]);
    }
    
    // free(unsorted);
    free(size);
    if (argc > 1) {
        fclose(fp);
    }

    return 0;
}
