#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "thread.h"
#include "globals.h"
#include "constants_define.h"

void *add_string_with_delay(void *arg) {
    args* routine_arg = (args*) arg;
    int delay = routine_arg->delay;
    char *string = routine_arg->string;
    char *thread_name = routine_arg->thread_name;

    usleep(delay);
    if (log_flag == LOG_ENABLE) {
        printf("%s%s\n", "string was added to list: ", string);
    }
    int ret_code = add(list, string, thread_name);

    return (void*) ret_code;
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
int create_threads(int string_count, pthread_t *threads, args *thread_args, int *created_thread_count) {
    for (int i = 0; i < string_count; i++) {
        int create_status = try_create_thread(&threads[i], &thread_args[i], i);
        if (create_status == FAILED) {
            *created_thread_count = i;
            return FAILED;
        }
    }
    return SUCCESS;
}

// joining the thread and print exception messages in corresponding threads if any
int try_join_thread(pthread_t *thread, int thread_idx) {
    void *status;
    int code = pthread_join(*thread, (void**) &status);

    if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%d%s%s\n", "can't join thread ", thread_idx, ": ", error_message);

        return FAILED;
    }
    int routine_ret_code = (int) status;

    if (routine_ret_code != SUCCESS) {
        return FAILED;
    }

    return SUCCESS;
}

// joining specified number of threads
int join_threads(int threads_count, pthread_t *threads) {
    for (int i = 0; i < threads_count; i++) {
        int join_status = try_join_thread(&threads[i], i);
        if (join_status == FAILED) {return FAILED;}
    }
    return SUCCESS;
}

int terminate_threads(int threads_count, pthread_t *threads_space) {
    int join_status = join_threads(threads_count, threads_space);
    if (join_status == FAILED) {
        return FAILED;
    }
    free(threads_space);
    
    return SUCCESS;
}

int start_threads(int threads_count, pthread_t *threads_space, args *arg_arr) {
    int created_thread_count = 0;
    int create_status = create_threads(threads_count, threads_space, arg_arr, &created_thread_count);
    if (create_status == FAILED) {
        terminate_threads(created_thread_count, threads_space);

        return FAILED;
    }
    return SUCCESS;
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


#include "constants_undef.h"
