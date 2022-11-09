#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "list.h"
#include "tools.h"
#include "constants_define.h"

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

// allocate pthread_t structures array
int allocate_threads_space(int string_count, pthread_t **threads_space) {
    *threads_space = (pthread_t*) malloc(sizeof(pthread_t) * string_count);

    if (*threads_space == NULL) {
        // errno is setted
        perror("can't allocate memory for pthread_t structures");
        return FAILED;
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
    int code = pthread_mutexattr_init(&attribute);
    if (code != SUCCESS) {
        char* exception_message = strerror(code);
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

void destroy_mutex(pthread_mutex_t *mutex) {
    int code = pthread_mutex_destroy(mutex);

    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        fprintf(stderr, "%s%s\n", "FATAL, mutex destroy error: ", exception_message);

        // terminate the process
        exit(FAILED);
    }
}

// free list and destroy list mutex; returns exception code (if any were occured) or SUCCESS code
int destroy_list(LinkedList *list) {
    check_list_consistency(list);
    int code = SUCCESS;

    code = free_list(list);
    destroy_mutex(list->list_mutex);

    return code;
}

// init list and mutex
int create_list(LinkedList *list, pthread_mutex_t *mutex) {
    int code = init_mutex(mutex);
    if (code != SUCCESS) {return FAILED;}
    init_list(list, mutex);

    return SUCCESS;
}

void print_args_info() {
    printf("possible arg values:\n");
    printf("arg='1': log adding items to the list.\n");
    printf("arg='0': don't log adding items to the list.\n");
}

// check string on not digit characters consistence
int has_not_digit(char* str)  {
    size_t arg_len = strlen(str);

    for (size_t i = 0;  i < arg_len; i++) {
        if (!(str[i] >= '0' && str[i] <= '9')) {
            return 1;
        }
    }
    return 0;
}

// check cast of string to int (terminate the process if fails)
int try_cast_arg_to_int(char* str) {
    // check is there not digit characters in string
    if (has_not_digit(str)) {
        printf("invalid arg: %s\n", str);
        // finish the process by exit() function
        exit(FAILED);
    }
    // check 0 input arg value
    size_t arg_len = strlen(str);
    if (arg_len == 1) {
        if (str[0] == '0') {
            return 0;
        }
    }
    // strtol() returns 0 if parse int was failed
    long int in_arg_value = strtol(str, NULL, BASE);
    if (in_arg_value == CAST_INT_FAILED || in_arg_value == LONG_MIN || in_arg_value == LONG_MAX) {
        printf("invalid arg: %s\n", str);
        // finish the process by exit() function
        exit(FAILED);
    }
    return (int) in_arg_value;
}

int parse_log_state(int argc, char *argv[]) {
    if (argc != ARGS_COUNT) {
        printf("%s%d%s\n","invalid args count - ", argc - 1, " should be 1:");
        print_args_info();

        return FAILED;
    }
    char *arg_str = argv[ARG_IDX];
    int arg = try_cast_arg_to_int(arg_str);

    if (arg == LOG_ENABLE || arg == LOG_DISABLE) {
        return arg;
    }
    printf("%s%d\n", "invalid log_flag value: ", arg);
    print_args_info();

    return FAILED;
}

#include "constants_undef.h"
