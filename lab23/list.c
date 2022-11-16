#include <string.h>
#include <stdio.h>

#include "constants_define.h"
#include "list.h"
#include "mutex.h"

void print_list_exception(char* operation, const char* info) {
    if (info != EMPTY_MSG) {
        fprintf(stderr, "%s%s%s\n", info, ": ", operation);
    }
}

LinkedList *create_list(const char *error_case_info) {
    char error_info[ERR_INFO_MAX_SIZE];
    if (error_case_info != EMPTY_MSG) {
        strcpy(error_info, error_case_info);
    }

    LinkedList *list = (LinkedList*) malloc(sizeof(LinkedList));
    if (list == NULL) {
        print_list_exception("create list", error_info);
        perror("can't create list");
        
        return NULL;
    }
    pthread_mutex_t *list_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    if (list_mutex == NULL) {
        free(list);
        print_list_exception("create list", error_info);
        perror("can't create list mutex");
        
        return NULL;
    }
    int code = init_mutex(list_mutex);
    if (code != SUCCESS) {
        free(list);
        free(list_mutex);

        return NULL;
    }
    list->list_mutex = list_mutex;
    list->head = NULL;
    list->tail = NULL;

    return list;
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
        int empty = is_empty(list);
        int not_empty = is_not_empty(list);
        // check list consistensy
        if (empty != SUCCESS && not_empty != SUCCESS) {
            status = FAILED;
        }
    }
    if (status != SUCCESS) {
        fprintf(stderr, "FATAL: list is invalid");
        exit(FAILED);
    }
}

int add(LinkedList *list, char *adding_string, const char *error_case_info) {
    check_list_consistency(list); // check list consistency

    char error_info[ERR_INFO_MAX_SIZE];
    if (error_case_info != EMPTY_MSG) {
        strcpy(error_info, error_case_info);
    }    
    pthread_mutex_t *list_mutex = list->list_mutex;

    // lock mutex for synchronization:
    int code = pthread_mutex_lock(list_mutex);
    if (code != SUCCESS) {
        print_list_exception("add item", error_info);
        print_lock_error_message(code);

        return FAILED;
    }
    Node *new_node = (Node*) malloc(sizeof(Node));

    if (new_node == NULL) {
        print_list_exception("add item", error_info);
        perror("can't create new node");

        return FAILED;   // errno is set
    }

    size_t string_length = strlen(adding_string);
    char* string_buffer = (char*) malloc(sizeof(char) * string_length + 1);

    if (string_buffer == NULL) {
        print_list_exception("add item", error_info);
        perror("can't allocate string ");

        return FAILED;
    }
    strcpy(string_buffer, adding_string);

    // init new node
    new_node->string = string_buffer;
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
        print_list_exception("add item", error_info);
        print_unlock_error_message(code);

        return FAILED;
    }

    return SUCCESS;
}

int print_list(LinkedList *list, const char *error_case_info) {
    check_list_consistency(list); // check list consistency

    char error_info[ERR_INFO_MAX_SIZE];
    if (error_case_info != EMPTY_MSG) {
        strcpy(error_info, error_case_info);
    }
    pthread_mutex_t *list_mutex = list->list_mutex;

    // lock mutex for synchronization
    int code = pthread_mutex_lock(list_mutex);
    if (code != SUCCESS) {
        print_list_exception("print list", error_info);
        print_lock_error_message(code);

        return FAILED;
    }

    Node *cur_item = list->head;

    while (cur_item != NULL) {
        printf("%s\n", cur_item->string);
        cur_item = cur_item->next;
    }

    // unlock mutex for synchronization:
    code = pthread_mutex_unlock(list_mutex);
    if (code != SUCCESS) {
        print_list_exception("print list", error_info);
        print_unlock_error_message(code);

        return FAILED;
    }

    return SUCCESS;
}

int free_list(LinkedList *list, const char *error_case_info) {
    check_list_consistency(list); // check list consistency

    char error_info[ERR_INFO_MAX_SIZE];
    if (error_case_info != EMPTY_MSG) {
        strcpy(error_info, error_case_info);
    }
    pthread_mutex_t *list_mutex = list->list_mutex;

    // lock mutex for synchronization:
    int code = pthread_mutex_lock(list_mutex);
    if (code != SUCCESS) {
        print_list_exception("free list", error_info);
        print_lock_error_message(code);
        
        return FAILED;
    }

    Node *cur_item = list->head;
    Node *next_item;

    if (cur_item == NULL) {
        // nothing to free the list is empty
        return SUCCESS;
    }

    while (cur_item != NULL) {
        next_item = cur_item->next;
        free(cur_item->string);
        free(cur_item);
        cur_item = next_item;
    }

    // unlock mutex for synchronization:
    code = pthread_mutex_unlock(list_mutex);
    if (code != SUCCESS) {
        print_list_exception("free list", error_info);
        print_unlock_error_message(code);

        return FAILED;
    }

    return SUCCESS;
}

int destroy_list(LinkedList *list, const char *error_case_info) {
    check_list_consistency(list);
    
    char error_info[ERR_INFO_MAX_SIZE];
    if (error_case_info != EMPTY_MSG) {
        strcpy(error_info, error_case_info);
    }

    int code = free_list(list, error_info);
    if (code != SUCCESS) {
        print_list_exception("destroy list", error_info);

        return FAILED;
    }
    code = pthread_mutex_destroy(list->list_mutex);
    if (code != SUCCESS) {
        print_list_exception("destroy list", error_info);
        print_destroy_error_message(code);

        return FAILED;
    }
    free(list->list_mutex);
    free(list);
}

#include "constants_undef.h"
