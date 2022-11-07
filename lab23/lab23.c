#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "tools.h"
#include "list.h"
#include "constants_define.h"

// thread shared resourses:
int log_flag = LOG_DISABLE;  
LinkedList list;

// thread's routine: adding new item to the list with specified delay
void *add_string_with_delay(void *arg) {
    args* routine_arg = (args*) arg;
    int delay = routine_arg->delay;
    char *string = routine_arg->string;

    usleep(delay);
    if (log_flag == LOG_ENABLE) {
        printf("%s%s\n", "string was added to list: ", string);
    }
    int ret_code = add(&list, string);

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
        char *exception_message = strerror(thread_ret_code);
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

int main(int argc, char *argv[]) {
    int string_count = 0;
    args arg_arr[MAX_STRING_COUNT];
    pthread_t *threads_space;
    pthread_mutex_t mutex;
    
    // parse log flag and set it:
    int status = parse_log_state(argc, argv);
    if (status == FAILED) {return FAILED;}
    if (status == LOG_ENABLE) {log_flag = LOG_ENABLE;}

    // read strings:
    printf("enter the string sequence:\n");
    string_count = read_strings(arg_arr);

    if (string_count == FAILED) {return FAILED;}
    if (string_count == 0) {
        printf("Nothing to sort: no strings were entered.\n");

        return SUCCESS;
    }
    printf("%s%d%s","The reading is over. Totally read strings ", string_count, "\n");

    // allocate thread's space:
    int allocate_status = allocate_threads_space(string_count, &threads_space);
    if (allocate_status == FAILED) {return FAILED;}

    // create list:
    int code = create_list(&list, &mutex);
    if (code != SUCCESS) {
        free(threads_space);
        return FAILED;
    }
    printf("\nstart adding items...\n");

    // create threads:
    int create_status = create_threads(string_count, threads_space, arg_arr);
    if (create_status == FAILED) {
        free(threads_space);
        return FAILED;
    }

    // joining all threads:
    int join_status = join_threads(string_count, threads_space);
    if (join_status == FAILED) {
        free(threads_space);
        return FAILED;
    }
    free(threads_space);

    printf("\nlist creating is completed successfully.\n");

    printf("strings in sorted order:\n");
    print_list(&list);

    code = destroy_list(&list);
    if (code != SUCCESS) {
        char *error_message = strerror(code);
        fprintf(stderr, "%s%s\n", "can't destroy a list: ", error_message);

        return FAILED;
    }

    return SUCCESS;
}

#include "constants_undef.h"
