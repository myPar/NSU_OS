#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// list itself
typedef struct _LinkedList
{
    pthread_mutex_t *list_mutex;
    Node *head;
    Node *tail;
} LinkedList;

// list's node
typedef struct _Node
{
    char *string;
    Node *next;
} Node;

// initialize list with initial state and with pre-initialized mutex
int init_list(LinkedList *list, pthread_mutex_t *mutex_ptr);
// check does list empty
int is_empty(LinkedList *list);
// check does list has any items
int is_not_empty(LinkedList *list);
// check list consistency state terminate process if consistency is broken
void check_list_consistency(LinkedList *list);
// add new item to the list
int add(LinkedList *list, char *adding_string);
// print list items
int print_list(LinkedList *list);
// free memory from list items
int free_list(LinkedList *list);
