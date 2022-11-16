#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mutex.h"
#include "constants_define.h"

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
        
        free_mutex_attribute(&attribute);

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

void print_lock_error_message(int code) {
    char *exception_message = strerror(code);
    fprintf(stderr, "%s%s\n", "can't lock mutex: ", exception_message);
}

void print_unlock_error_message(int code) {
    char *exception_message = strerror(code);
    fprintf(stderr, "%s%s\n", "can't unlock mutex: ", exception_message);
}

void print_destroy_error_message(int code) {
    char *exception_message = strerror(code);
    fprintf(stderr, "%s%s\n", "can't destroy mutex: ", exception_message);
}

#include "constants_undef.h"