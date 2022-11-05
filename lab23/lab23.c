#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_STRING_SIZE 100
#define DELTA_TIME 100000
#define MAX_STRING_COUNT 100
#define SUCCESS 0
#define FAILED -1

//--------- Linked List structrues and funtions definitions --------------//
typedef struct _LinkedList
{
    pthread_mutex_t *list_mutex;
    Node *head;
    Node *tail;
} LinkedList;

typedef struct _Node
{
    char *string;
    Node *next;
} Node;

// init list with initialized mutex
int init_list(LinkedList *list, pthread_mutex_t *mutex_ptr) {
    if (list != NULL && mutex_ptr != NULL) {
        // init mutex
        list->head = NULL;
        list->tail = NULL;
        list->list_mutex = mutex_ptr;
    }
}

int is_empty(LinkedList *list`) {
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
    if (list == NULL) {
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
        return;
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

//--------- main program functions and structures --------------//

typedef struct _arg {
    char *string;
    int delay; // in microseconds
    LinkedList *list;
} arg;

// thread's routine: adding new item to the list with specified delay
void *add_string_with_delay(void *arg) {
    args* routine_arg = (args*) arg;
    int delay = routine_arg->delay;
    char *string = routine_arg->string;
    LinkedList *list = routine_arg->list;

    usleep(delay);
    printf("%s\n", string);
    int ret_code = add(list);

    return (void*) ret_code;
}

// reading input strings:
int read_strings(args *arg_arr) {
    int string_count = 0;

    while (string_count < MAX_STRING_COUNT) {
        int cur_idx = string_count;
        char *buffer = arg_arr[cur_idx].string;
        int *time_delay = &arg_arr[cur_idx].delay;

        char *result_ptr = fgets(buffer, MAX_STRING_SIZE, stdin);

        if (result_ptr == NULL) { // error accured/EOF and no bytes were entered
              //check reading error
            int error_status = ferror(stdin);
            if (error_status != SUCCESS) {
                fprintf(stderr, "an error occurred while reading from stdin\n");

                return FAILED;
            }
            // check EOF
            int eof_status = feof(stdin);
            if (eof_status != SUCCESS) {
                // no data were transfered to buffer so just break from reading loop
                printf("\nEOF is reached, the reading is over\n");
                break;  
            }
        }
        // check just '\n' was entered (some data were transfered to buffer)
        if (buffer[0] == '\n') {
            // the reading is over
            break;
        }
        int string_size = (int) strlen(buffer);

        // reset '\n' character on '\0' if it is exists
        if (buffer[string_size - 1] == '\n') {
            buffer[string_size - 1] = '\0';
        }
        // init time delay arg
        *time_delay = DELTA_TIME * string_size;
        string_count++;
    }

    return string_count;
}

int allocate_threads_space(int string_count, pthread_t **threads_space) {
    *threads_space = (pthread_t*) malloc(sizeof(pthread_t) * string_count);

    if (*threads_space == NULL) {
        // errno is setted
        perror("can't allocate memory for pthread_t structures");
        return FAILED;
    }
    return SUCCESS;
}

// creating a thread with exception handling
int try_create_thread(pthread_t *thread, args* thread_arg, int thread_idx) {
  int code = pthread_create(thread, NULL, add_string_with_delay, thread_arg);

  if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%d%s%s\n", "can't create a thread ", thread_idx, ":", error_message);

        return FAILED;
  }
  return SUCCESS;
}

//  starting thread's routines
int create_threads(int string_count, pthread_t *threads, args *thread_args) {
    for (int i = 0; i < string_count; i++) {
      int create_status = try_create_thread(&threads[i], &thread_args[i], i);
      if (create_status == FAILED) {return FAILED;}
    }
    return SUCCESS;
}

// joining the thread and print exception messages in corresponding threads if any
int try_join_thread(pthread_t *thread, int thread_idx) {
    int thread_ret_code;
    int code = pthread_join(*thread, (void**) &thread_ret_code);

    if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%d%s%s\n", "can't join thread ", thread_idx, ": ", error_message);

        return FAILED;
    }
    // print thread routine exception message
    if (thread_ret_code != SUCCESS) {
        int exception_message = strerror(thread_ret_code);
        fprintf(stderr, "%s%d%s%s\n", "exception in thread ", thread_idx, ": ", exception_message);
    }
    return SUCCESS;
}

// joining all threads
int join_threads(int string_count, pthread_t *threads) {
    for (int i = 0; i < string_count; i++) {
        int join_status = try_join_thread(&threads[i], i);
        if (join_status == FAILED) {return FAILED;}
    }
    return SUCCESS;
}

void free_mutex_attribute(pthread_mutexattr_t *attribute) {
    int code = pthread_mutexattr_destroy(attribute);

    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        fprintf(stderr, "%s%s\n", "FATAL, mutex attr destroy error: ", exception_message);

        // terminate the process
        exit(FAILED);
    }
}

// init ERROR_CHECK mutex
int init_mutex(pthread_mutex_t *mutex) {
    pthread_mutexattr_t attribute;

    // init mutex attribute
    int code = pthread_mutex_attr_init(&attribute);
    if (code != SUCCESS) {
        char* exception_message = strerror(code1);
        fprintf(stderr, "%s%s\n", "mutex atrr init error: ", exception_message);
        
        return FAILED;
    }
    // set ERRORCHECK type
    code = pthread_mutexattr_settype(&attribute, PTHREAD_MUTEX_ERRORCHECK);
    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        fprintf(stderr, "%s%s\n", "mutex atrr set type error: ", exception_message);
        
        return FAILED;
    }
    // init mutex
    code = pthread_mutex_init(mutex, &attribute);
    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        fprintf(stderr, "%s%s\n", "mutex init error: ", exception_message);
        
        free_mutex_attribute(&attribute);

        return FAILED;
    }
    free_mutex_attribute(&attribute);

    return SUCCESS;
}

int destroy_mutex(pthread_mutex_t *mutex) {
    int code = pthread_mutex_destroy(mutex);

    if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%s\n","can't destroy mutex: ", error_message);

        return FAILED;
    }
    return SUCCESS;
}

int main() {
    int string_count = 0;
    args arg_arr[MAX_STRING_COUNT];
    pthread_t *threads_space;

    printf("enter the string sequence:\n");
    string_count = read_strings(arg_arr);

    if (string_count == FAILED) {return FAILED;}
    if (string_count == 0) {
        printf("Nothing to sort: no strings were entered.\n");

        return SUCCESS;
    }
    printf("%s%d%s","The reading is over. Totally read strings ", string_count, "\n");

    // allocate thread's space
    int allocate_status = allocate_threads_space(string_count, &threads_space);
    if (allocate_status == FAILED) {return FAILED;}

    printf("\nstart sorting...\n");

    // create threads
    int create_status = create_threads(string_count, threads_space, arg_arr);
    if (create_status == FAILED) {
        free(threads_space);
        return FAILED;
    }

    // joining all threads
    int join_status = join_threads(string_count, threads_space);
    if (join_status == FAILED) {
        free(threads_space);
        return FAILED;
    }
    free(threads_space);

    printf("\nthe sorting is completed successfully.\n");

    return SUCCESS;
}

#undef MAX_STRING_SIZE
#undef DELTA_TIME
#undef MAX_STRING_COUNT
#undef SUCCESS
#undef FAILED
