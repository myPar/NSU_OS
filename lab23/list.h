#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// list's node
typedef struct _Node Node;
struct _Node
{
    char *string;
    Node *next;
};

// list itself
typedef struct _LinkedList LinkedList;
struct _LinkedList
{
    pthread_mutex_t *list_mutex;
    Node *head;
    Node *tail;
};
//char *error_case_info - can be info about calling thread, for example
// return empty list ptr or NULL if error ocurred
LinkedList *create_list(const char *error_case_info);
int add(LinkedList *list, char *adding_string, const char *error_case_info);
int print_list(LinkedList *list, const char *error_case_info);
// free memory from list items
int free_list(LinkedList *list, const char *error_case_info);
// free list nodes, destroy and free list mutex, free list structure
int destroy_list(LinkedList *list, const char *error_case_info);
