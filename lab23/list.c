#include "list.h"
#include "constants_define.h"

//--------- Linked List funtions definitions --------------//

// init list with initialized mutex
int init_list(LinkedList *list, pthread_mutex_t *mutex_ptr) {
    if (list != NULL && mutex_ptr != NULL) {
        // init mutex
        list->head = NULL;
        list->tail = NULL;
        list->list_mutex = mutex_ptr;
    }
}

int is_empty(LinkedList *list) {
    if (list->head == NULL && list->tail == NULL) {
        return SUCCESS;
    }
    return FAILED;
}

int is_not_empty(LinkedList *list) {
    if (list->head != NULL && list->tail != NULL) {
        return SUCCESS;
    }
    return FAILED;
}

// terminates the process if list is not in consistent state (to prevent using of incorrect list)
void check_list_consistency(LinkedList *list) {
    int status = SUCCESS;
    if (list == NULL || list->list_mutex == NULL) {
        status = FAILED;
    }
    else {
        int is_empty = is_empty(list);
        int is_not_empty = is_not_empty(list);
        // check list consistensy
        if (is_empty != SUCCESS && is_not_empty != SUCCESS) {
            status = FAILED
        }
    }
    if (status != SUCCESS) {
        fprintf(stderr, "FATAL: list is invalied");
        exit(FAILED);
    }
}

// adding new item to the end of list (synchronized)
int add(LinkedList *list, char *adding_string) {
    check_list_consistency(list); // check list consistency

    pthread_mutex_t *list_mutex = list->list_mutex;

    // lock mutex for synchronization:
    int code = pthread_mutex_lock(list_mutex);
    if (code != SUCCESS) {
        return code;
    }
    // allocate memory for new node:
    Node *new_node = (Node*) malloc(sizeof(Node));
    
    if (new_node == NULL) {
        return errno;   // errno is set
    }
    // init new node
    new_node->string = adding_string;
    new_node->next = NULL;
    
    Node *head = list->head;
    Node *tail = list->tail;

    // add node to list:
    if (head == NULL) {
        list->head = new_node;
        list->tail = new_node;
    }
    else {
        list->tail->next = new_node;
        list->tail = new_node;
    }

    // unlock mutex for synchronization:
    code = pthread_mutex_unlock(list_mutex);
    if (code != SUCCESS) {
        return code;
    }

    return SUCCESS;
}

// print all list items
int print_list(LinkedList *list) {
    check_list_consistency(list); // check list consistency
    
    pthread_mutex_t *list_mutex = list->list_mutex;

    // lock mutex for synchronization
    int code = pthread_mutex_lock(list_mutex);
    if (code != SUCCESS) {
        return code;
    }

    Node *cur_item = list->head;

    while (cur_item != NULL) {
        printf("%s\n", cur_item->string);
        cur_item = cur_item->next;
    }

    // unlock mutex for synchronization:
    code = pthread_mutex_unlock(list_mutex);
    if (code != SUCCESS) {
        return code;
    }

    return SUCCESS;
}

// free list memory from list nodes (without mutex destroying!)
int free_list(LinkedList *list) {
    check_list_consistency(list); // check list consistency

    Node *cur_item = list->head;
    Node *next_item;

    if (cur_item == NULL) {
        // nothing to free the list is empty
        return SUCCESS;
    }
    pthread_mutex_t *list_mutex = list->list_mutex;
    
    // lock mutex for synchronization:
    int code = pthread_mutex_lock(list_mutex);
    if (code != SUCCESS) {
        return code;
    }

    while (cur_item != NULL) {
        next_item = cur_item->next;
        free(cur_item);
        cur_item = next_item;
    }

    // unlock mutex for synchronization:
    code = pthread_mutex_unlock(list_mutex);
    if (code != SUCCESS) {
        return code;
    }

    return SUCCESS;
}

#include "constants_undef.h"
