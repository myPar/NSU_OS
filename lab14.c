#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <semaphore.h>

#define SUCCESS 0
#define FAILED 1
#define CORRECT_STATUS 0
#define EXCEPTION_STATUS -1

// threads id's
#define PARENT_ID 0
#define CHILD_ID 1
// semaphores count
#define SEM_COUNT 2

const int iteration_count = 10;
// semaphores array
sem_t semaphores[SEM_COUNT];

// structure represents thread args
typedef struct _args {
        char *string;
        int thread_id;  // specified parent or child thread use this arg (0 - parent, 1 - child)
} args;

// Parent args
args parent_args;
// child args
args child_args;

void *print(void* param) {
    args *arguments = (args*) param;
    int id = arguments->thread_id;
    char* string = arguments->string;

    sem_t *my_sem = &semaphores[id];
    sem_t *other_sem = &semaphores[(id + 1) % 2];

    for (int i = 0; i < iteration_count; i++) {
        if (sem_wait(my_sem) == EXCEPTION_STATUS) {
            // return exception code, errno is set
            return errno;
        }
        printf(string);

        if (sem_post(other_sem) == EXCEPTION_STATUS) {
            // return exception code, errno is set
            return errno;
        }
    }
    return SUCCESS;
}
// free resources from semaphores funcion
void destroy_semphores() {
    int status = sem_destroy(&semaphores[PARENT_ID]);
    if (status == EXCEPTION_STATUS) {
        perror("Fatal: can't destroy the first semaphore");
        exit(FAILED);
    }
    status = sem_destroy(&semaphores[CHILD_ID]);
    if (status == EXCEPTION_STATUS) {
        perror("Fatal: can't destroy the second semaphore");
        exit(FAILED);
    }
}
// function inits both semaphores, prints exception messages, returs 
int init_semaphores() {
    int status = sem_init(&semaphores[PARENT_ID], 0, 1);
    
    if (status == EXCEPTION_STATUS) {
        // errno is setted, print exception message
        perror("can't init the first semaphore");

        return FAILED;
    }
    status = sem_init(&semaphores[CHILD_ID], 0, 0);
    
    if (status == EXCEPTION_STATUS) {
        // errno is setted, print exception message
        perror("can't init the second semaphore");

        return FAILED;
    }
    return SUCCESS;
}

// check threads routine return code function
int check_results(int child_ret_code, int parent_ret_code) {
    int status = SUCCESS;
    if (parent_ret_code != SUCCESS) {
        char* exception_message = strerror(parent_ret_code);
        fprintf(stderr, "%s%s\n", "Error in parent thread: ", exception_message);
        status = FAILED;
    }
    if (child_ret_code != SUCCESS) {
        char* exception_message = strerror(child_ret_code);
        fprintf(stderr, "%s%s\n", "Error in child thread: ", exception_message);
        status = FAILED;
    }
    return status;
}

int main() {
    // init semaphores:
    if (init_semaphores() != SUCCESS) {
        return FAILED;
    }
    // init args:
    parent_args.string = "Hello from Parent";
    parent_args.thread_id = PARENT_ID;

    child_args.string = "Hello from Child";
    child_args.thread_id = CHILD_ID;

    pthread_t thread_id;
    int code = pthread_create(&thread_id, NULL, print, &child_args);
    
    // check create child thread status:
    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        fprintf(stderr, "%s%s\n", "can't create child thread: ", exception_message);
        destroy_semphores();

        return FAILED;
    }
    int parent_ret_code = print(&parent_args);
    int child_ret_code;
    void *child_result;

    // join child thread:
    int code = pthread_join(*thread_id, &child_result);
    if (code != SUCCESS) {
        char* exception_message = strerror(code);
        fprintf(stderr, "%s%s\n", "Can't join thread: ", exception_message);

        destroy_semphores();

        return FAILED;
    }
    child_ret_code = child_result;

    // checking ret codes for Parent and Child routine
    if (check_results(child_ret_code, parent_ret_code) != SUCCESS) {
        destroy_semphores();

        return FAILED;    
    }

    // free resourses
    destroy_semphores();

    return SUCCESS;
}
#undef SUCCESS
#undef FAILED
#undef CORRECT_STATUS
#undef EXCEPTION_STATUS
#undef SEM_COUNT