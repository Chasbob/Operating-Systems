/**
 *  Source file for a linked list in C
 *
 * @authors:   		Michael Denzel
 * @creation_date:	2016-09-05
 * @contact:		m.denzel@cs.bham.ac.uk
 */

//standard includes

// TODO: Add the includes you need

//own includes
#include "linkedlist.h"
#include <stdlib.h>
#include <stdio.h>

// TODO: Implement those methods!

node *getNthNode(node *n, unsigned int index) {
    if (index == 0) {
        return n;
    } else if (n->next != NULL) {
        return getNthNode(n->next, index - 1);
    } else {
        return NULL;
    }
}

int get(list *l, unsigned int index) {
    return getNthNode(l->head, index)->value;
}

//int getNext(node *n, unsigned int index) {
//    if (index == 0) {
//        return n->value;
//    } else if (n->next != NULL) {
//        return getNext(n->next, index - 1);
//    } else {
//        return NULL;
//    }
//}

node *newNode(int value) {
    struct node *n = (node *) malloc(sizeof(node));
//    int *newValue = malloc(sizeof(int));
//    *newValue = value;
    n->value = value;
    return n;
}

int prepend(list *l, int data) {
    struct node *newHead = newNode(data);
    newHead->next = l->head;
    l->head = newHead;
    return data;
}

int append(list *l, int data) {
    struct node *newLast = newNode(data);
    struct node *current = l->head;
    while (current != NULL) {
        if (current->value == NULL) {
            current->value = data;
            return 0;
        } else if (current->next == NULL) {
            current->next = newLast;
            return 0;
        }
    }
    return data;
}


int remove_element(list *l, unsigned int index) {
    struct node *current = l->head;
    struct node *prev = NULL;
    while (current != NULL && index >= 0) {
        if (index == 0) {
            if (current->next == NULL) {
                if (prev == NULL) {
                    free(current);
                    return 1;
                } else {
                    prev->next = NULL;
                    free(current);
                    return 1;
                }
            }else{
                
            }
        }
    }
    if (index == 0) {
        node *head = l->head;
        int value = head->value;
        if (l->last == head) {
            destroy(l);
        } else {
            l->head = head->next;
            free(head);
        }
        return value;
    } else {
        node *before = getNthNode(l->head, index - 1);
        if (before == NULL) {
            printf("Out of bounds.");
            exit(2);
        }
        node *n = getNthNode(l->head, index);
        before->next = n->next;
        free(n);
    }

}

int insert(list *l, unsigned int index, int data) {
    if (index == 0) {
        return prepend(l, data);
    } else {
        node *before = getNthNode(l->head, index - 1);
        node *n = newNode(data);
        n->next = before->next;
        before->next = n;
    }
}

void print_list(list *l) {
    node *current = l->head;
    while (current != NULL) {
        printf("%i\n", current->value);
        current = current->next;
    }
}

void init(list *l) {
    l = malloc(sizeof(list));
    if (l == NULL) {
        printf("malloc error.");
        exit(1);
    }
    struct node *n = malloc(sizeof(node));
    if (n == NULL) {
        printf("malloc error.");
        exit(1);
    }
    n->value = NULL;
    n->next = NULL;
    l->head = n;
}

void destroy(list *l) {
}
