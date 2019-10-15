//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//
//void swap(char *xp, char *yp) {
//    char *temp = (char *) malloc(sizeof(xp));
//    strcpy(temp, xp);
//    strcpy(xp, yp);
//    strcpy(yp, temp);
//    free(temp);
//}
//
//void bubbleSort(char **arr, int n) {
//    int i, j;
//    for (i = 0; i < n - 1; i++)
//
//        // Last i elements are already in place
//        for (j = 0; j < n - i - 1; j++)
//            if (strcmp(*(arr + j), *(arr + j + 1)) > 0) {
//                swap(*(arr + j), *(arr + j + 1));
//            }
//}
//
//void addChar(char *line, int *length, char c) {
////    int length = *lengthP;
//// use realloc on null
//    if (*length == 0) {
//        line = (char *) malloc(sizeof(char *));
//        if (line == NULL) {
//            printf("Error allocating memory.\n");
//            exit(1);
//        }
////        line = newLine;
//        *line = c;
//    } else {
//        char **ptr = (char **) realloc(line, sizeof(char *) * (++(*length) + 1));
//        if (*ptr == NULL) {
//            printf("Error reallocating memory.\n");
//            exit(1);
//        }
//        *(line + ++(*length)) = c;
//    }
////    *lengthP = length;
//}
//
//void newLine(char **lines, int *lineNO, int *lineLength, char c) {
//    addChar(*(lines + (*lineNO)++), lineLength, (char) c);
//    char **ptr = (char **) realloc(lines, sizeof(char **) * ((*lineNO) + 1));
//    if (ptr == NULL) {
//        printf("Error reallocating memory.\n");
//        exit(1);
//    }
//    *lineLength = 0;
//}
//
//char *parse(FILE *fp) {
//    char *chars = (char *) malloc(sizeof(char *));
//    while (1) {
//        char c = (char) fgetc(fp);
//        if (feof(fp)) {
//            // do thing
//            break;
//        }
//
//    }
//}
//
//int *parseFile(FILE *fp, char **lines) {
//    int *lineNO = (int *) malloc(sizeof(int *));
//    int lineLength = 0;
//    while (1) {
//        char c = (char) fgetc(fp);
//        if (feof(fp)) {
//            newLine(lines, lineNO, &lineLength, '\n');
//            break;
//        }
//        if (c == '\n') {
//            newLine(lines, lineNO, &lineLength, c);
//        } else {
//            char *line = *(lines + *lineNO);
//            addChar(line, &lineLength, c);
//        }
////        printf("%s\n",*lines);
//    }
//    fclose(fp);
//    (*lineNO)++;
//    return lineNO;
//}
//
//
//int main(int argc, char **argv) {
//    // TODO: Implement your code here!
//    FILE *fp;
//    char **lines = (char **) malloc(sizeof(char **));
//
//    if (argc > 1) {
//        fp = fopen(argv[1], "r");
//        if (fp == NULL) {
//            fprintf(stderr, "cannot open %s\n", argv[1]);
//            return 1;
//        }
//    } else {
//        fp = stdin; /* read from standard input if no argument on the command line */
//    }
//    int *count = parseFile(fp, lines);
//    printf("%i", *count);
////    bubbleSort(lines, (*count) - 1);
////    for (int i = 0; i < *count; ++i) {
////        printf("`%s`", *(lines + i));
////    }
////    printf("%i lines\n", *count);
//
//
//}
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int INITIAL_MAX_LINES = 2;
const int MAX_LINES_INC = 2;

const int INITIAL_MAX_LINE_LENGTH = 2;
const int MAX_LINE_LENGTH_INC = 2;

int cmpstr(void const *a, void const *b) {
    char const *aa = (char const *) a;
    char const *bb = (char const *) b;

    return strcmp(aa, bb);
}

void swap(char *xp, char *yp) {
    char *temp = xp;
    xp = yp;
    yp = temp;
//    strcpy(temp, xp);
//    strcpy(xp, yp);
//    strcpy(yp, temp);
//    free(temp);
}

void bubbleSort(char *arr[], int n) {
    int i, j;
    for (i = 0; i < n - 1; i++)

        // Last i elements are already in place
        for (j = 0; j < n - i - 1; j++)
            if (strcmp(arr[j], arr[j + 1]) > 0) {
//                swap(arr[j], arr[j + 1]);
                char *temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

int main(int argc, char *argv[]) {
    int maxlength = 0;
    int nlines = 0, i;
    FILE *fp;

    char **inputFile, *buffer;
    int max_lines, c, buflen, bufpos, end_of_line;


    if (argc > 1) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            fprintf(stderr, "cannot open %s\n", argv[1]);
            return 1;
        }
    } else {
        fp = stdin; /* read from standard input if no argument on the command line */
    }
//    if (argc < 2) {
//        printf("No enough arguments.\n");
//        return -1;
//    }

    max_lines = INITIAL_MAX_LINES;

    inputFile = (char **) malloc(max_lines * sizeof(char *));
    if (fp == 0) {
        fprintf(stderr, "Cannot open file!\n");
        return -1;
    } else {
        /* Start with a buffer. */
        bufpos = 0;
        buflen = INITIAL_MAX_LINE_LENGTH;
        buffer = (char *) malloc(buflen * sizeof(char *));

        c = 0;
        do {

            end_of_line = 0;

            c = fgetc(fp);
            if (c == EOF) {
                end_of_line = 2;
            } else if (c == '\n' || c == '\r') {
                end_of_line = 1;
            }
//            else {
            /* Put this character in the buffer. */
            /* But check if we have enough memory first! */
            /* Leave room for the null character at the end. */
            if (bufpos >= buflen - 1) {
                buflen += MAX_LINE_LENGTH_INC;
                buffer = (char *) realloc(buffer, buflen * sizeof(char));
            }
            if (end_of_line == 1) {
                buffer[bufpos] = '\n';
//                printf("1%c", buffer[bufpos]);
            } else if (end_of_line == 2) {
//                end_of_line = 0;
                if (bufpos != 0) {
                    buffer[bufpos] = '\n';
                }
                else {
                    buffer[bufpos] = 0;
                }

//                inputFile[nlines] = buffer;
            } else {
                buffer[bufpos] = (char) c;
            }
//            printf("%c", buffer[bufpos]);
            bufpos++;
//            }

            if (end_of_line) {
                /* Remember this line and get a new buffer. */
                /* Check if we need more memory. */
                if (nlines >= max_lines) {
                    max_lines += MAX_LINES_INC;
                    inputFile = (char **) realloc(inputFile, max_lines * sizeof(char *));
                }

                /* Null terminate the buffer.*/
                buffer[bufpos++] = 0;

                inputFile[nlines] = buffer;
                nlines++;

                bufpos = 0;
                if (buflen > maxlength) {
                    maxlength = buflen;
                }
                buflen = INITIAL_MAX_LINE_LENGTH;
                buffer = (char *) malloc(buflen * sizeof(char *));
            }
        } while (c != EOF);
    }
//    printf("%i\n",maxlength);
//    qsort(inputFile,nlines,maxlength,cmpstr);
//    printf("%d lines\n", nlines);
//    for (i = 0; i < nlines; i++) {
//        char *s = inputFile[i];
//        printf("%s", s);
//    }
    bubbleSort(inputFile, nlines);
//    printf("\nSorted.\n");
    for (i = 0; i < nlines; i++) {
        char *s = inputFile[i];
        printf("%s", s);
    }
}
