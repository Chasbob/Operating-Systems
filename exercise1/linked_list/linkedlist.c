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

int pop_first(list *l) {
    node *next = NULL;
    node *current = l->head;
    if (current == NULL) {
        return -1;
    }
    if (current->next == NULL) {
        l->head = current->next;
        return 0;
    }
    next = current->next;
    free(current);
    l->head = next;
    return 0;
}


node *getNthNode(node *n, unsigned int index) {
    if (index == 0) {
        return n;
    } else if (n != NULL) {
        return getNthNode(n->next, index - 1);
    } else {
        return NULL;
    }
}

int get(list *l, unsigned int index) {
    node *got = getNthNode(l->head, index);
    return got->data;
//    if (got->data == -1) {
//        return -1;
//    } else {
//        return got->data;
//    }
}

//int getNext(node *n, unsigned int index) {
//    if (index == 0) {
//        return n->data;
//    } else if (n->next != NULL) {
//        return getNext(n->next, index - 1);
//    } else {
//        return NULL;
//    }
//}

node *newElement(int value) {
    node *n = malloc(sizeof(node));
    n->data = value;
    return n;
}

int push(struct list *head, int data) {
    node *new_node;
    new_node = malloc(sizeof(node));
    if (new_node == NULL) {
        return -1;
    }
    new_node->data = data;
    new_node->next = head->head;
    head->head = new_node;
    return 0;
}

int prepend(list *l, int data) {
    if (l->head->data == -1) {
        l->head->data = data;
        return 0;
    }
    return push(l, data);
}

int append(list *l, int data) {
    node *current = l->head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(node));
    current->next->data = data;
    current->next->next = NULL;
    return 1;
}


//int remove_by_index(list *l, int n) {
//
//
//}

int remove_element(list *l, unsigned int index) {
    unsigned int i = 0;
    node *current = l->head;
    node *temp_node = NULL;

    if (index == 0) {
        return pop_first(l);
    }

    for (i = 0; i < index - 1; i++) {
        if (current->next == NULL) {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    current->next = temp_node->next;
    free(temp_node);

    return 0;
}

//
//
int insert(list *l, unsigned int index, int data) {
    if (index == 0) {
        return push(l, data);
//        return prepend(l, data);
    } else {
        node *before = getNthNode(l->head, index - 1);
        if (before == NULL) {
            return -1;
        }
        node *n = newElement(data);
        n->next = before->next;
        before->next = n;
    }
    return 1;
}

void print_list(list *head) {
    node *current = head->head;
    if(current==NULL){
        printf("empty list\n");
        return;
    }

    while (current != NULL) {
        printf("%d ", current->data);
        current = current->next;
    }
    printf("\n");
}

void init(list *l) {
//    l = malloc(sizeof(list));
//    if (l == NULL) {
//        exit(1);
//    }
    l->head = malloc(sizeof(node));
    if (l->head == NULL) {
        exit(1);
    }
    l->head->data = -1;
    l->head->next = NULL;
}

void destroy(list *l) {
    node *current = l->head;
    node *prev = NULL;
    while (current != NULL) {
        prev = current;
        current = current->next;
        free(prev);
    }
}
