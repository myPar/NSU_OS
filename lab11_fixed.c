#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#define SUCCESS 0
#define FAILED 1
#define MUTEX_COUNT 3

pthreat_mutex_t mutexies[MUTEX_COUNT];
pthread_mutexattr_t mutex_attributes[MUTEX_COUNT];
int iteration_count = 10;
int init_state = 1;
int init_exception_status = 0;

// destroy first count mutex attributes
void free_attributes(int count) {
    assert(count <= MUTEX_COUNT);

    for (int i = 0; i < count; i++) {
        int code = pthread_mutexattr_destroy(&mutex_attributes[i]);
        
        if (code != SUCCESS) {
            char* exception_message = strerror(code);
            printf("%s%s\n", "FATAL, mutex attr destroy error: ", exception_message);

            // terminate the process
            exit(FAILED);
        }
    }
}

// destroy first count mutexies
void free_mutexies(int count) {
    assert(count <= MUTEX_COUNT);

    for (int i = 0; i < count; i++) {
        int code = pthread_mutex_destroy(&mutexies[i]);
        
        if (code != SUCCESS) {
            char* exception_message = strerror(code);
            printf("%s%s\n", "FATAL, mutex destroy error: ", exception_message);
            
            // terminate the process
            exit(FAILED);
        }
    }
}

// init mutex attributes objects
int init_attributes() {
    int code1;
    int code2;

    for (int i = 0; i < MUTEX_COUNT; i++) {
        //init mutex attributes with default values
        code1 = pthread_mutexattr_init(&mutex_attributes[i]);
        if (code1 != SUCCESS) {
            char* exception_message = strerror(code1);
            printf("%s%s\n", "mutex atrr init error: ", exception_message);
            // free resources (summary - i attributes)
            free_attributes(i);
            
            return FAILED;
        }
        // set type ERROR_CHECK
        code2 = pthread_mutexattr_settype(&mutex_attributes[i], PTHREAD_MUTEX_ERRORCHECK);
        if (code2 != SUCCESS) {
            char* exception_message = strerror(code2);
            printf("%s%s\n", "mutex atrr set type error: ", exception_message);
            // free resources (summary - i attributes)
            free_attributes(i);

            return FAILED;    
        }
    }
    return SUCCESS;
}

// init mutexies with initialized before mutex attributes
int init_mutexies() {
    for (int i = 0; i < MUTEX_COUNT; i++) {
        int code = pthread_mutex_init(&mutexies[i], &mutex_attributes[i]);
        
        if (code != SUCCESS) {
            char* exception_message = strerror(code);
            printf("%s%s\n", "mutexies init error: ", exception_message);
            
            // free resources (destroy all attributes, destroy i mutexies):
            free_attributes(MUTEX_COUNT);
            free_mutexies(i);
            
            return FAILED;            
        }
    }
    return SUCCESS;
}

// parent thread function
void* parent_routine() {
    int code;
    for (int i = 0; i < iteration_count; i++) {
        printf("Hello from Parent");
        code = pthread_mutex_lock(&mutexies[2]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_unlock(&mutexies[0]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_lock(&mutexies[1]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_unlock(&mutexies[2]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_lock(&mutexies[0]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_unlock(&mutexies[1]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_lock(&mutexies[2]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_unlock(&mutexies[2]);
        if (code != SUCCESS) {return code;}
    }
    code = pthread_mutex_unlock(&mutexies[0]);
    if (code != SUCCESS) {return code;}

    return SUCCESS;
}

// child thread function
void* child_routine() {
    int code;
    // initial lock
    code = pthread_mutex_lock(&mutexies[2]);
    if (code != SUCCESS) {
        init_exception_status = 1;
        return code;
    }

    init_state = 0;

    for (int i = 0; i < iteration_count; i++) {
        code = pthread_mutex_lock(&mutexies[0]);
        if (code != SUCCESS) {return code;}
        
        printf("Hello from Child");
        code = pthread_mutex_unlock(&mutexies[1]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_lock(&mutexies[2]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_unlock(&mutexies[0]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_lock(&mutexies[1]);
        if (code != SUCCESS) {return code;}

        code = pthread_mutex_unlock(&mutexies[2]);
        if (code != SUCCESS) {return code;}
    }
    code = pthread_mutex_unlock(&mutexies[1]);
    if (code != SUCCESS) {return code;}

    return SUCCESS;
}

int main() {
    // init attributes
    int code = init_attributes();
    if (code != SUCCESS) {
        return code;
    }
    // init mutexies
    code = init_mutexies();
    if (code != SUCCESS) {
        return code;
    }
    // free attributes objects after mutex initialization
    free_attributes(MUTEX_COUNT);

    // initial lock
    code = pthread_mutex_lock(&mutexies[0]);
    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        printf("%s%s\n", "mutexies lock error: ", exception_message);

        return FAILED;
    }
    // create child thread
    pthread_t thread_id;
    code = pthread_create(&thread_id, NULL, child_routine(), NULL);
    
    // parent tread wait till child thread will lock mutex
    while(init_state && !init_exception_status) {
        sleep(1);
    }
    void *result;
    if (init_exception_status) {
        // ignore ret status from child thread, no need to check join status
        // because this case is already has error
        code = pthread_join(thread_id, &result);
        if (code != SUCCESS) {
            char* exception_message = strerror(code);
            printf("%s%s\n", "Init state failed and can't join thread: ", exception_message);

            return FAILED;
        }
        code = result;
        char* exception_message = strerror(code);
        printf("%s%s\n", "Init state failed: ", exception_message);

        return FAILED;
    }
    int parent_ret_code;
    int child_ret_code;
    void *child_result;

    parent_ret_code = parent_routine();

    code = pthread_join(thread_id, &child_result);
    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        printf("%s%s\n", "Can't join thread: ", exception_message);

        return FAILED;
    }
    child_ret_code = child_result;
    // check threads
    if (child_ret_code != SUCCESS || parent_ret_code != SUCCESS) {
        if (parent_ret_code != SUCCESS) {
            char* exception_message = strerror(parent_ret_code);
            printf("%s%s\n", "Error in parent thread: ", exception_message);
        }
        if (child_ret_code != SUCCESS) {
            char* exception_message = strerror(child_ret_code);
            printf("%s%s\n", "Error in child thread: ", exception_message);
        }
        return FAILED;
    }
    
    // all mutexies are unlocked so can free the resources
    free_mutexies(MUTEX_COUNT);

    return SUCCESS;
}

#undef MUTEX_COUNT
#undef SUCCESS
#undef FAILED