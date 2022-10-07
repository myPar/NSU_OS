#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

// constants
#define CORRECT_EXIT_CODE 0 
#define EXCEPTION_EXIT_CODE 1   
#define INPUT_ARGS_COUNT 2
#define MAX_THREAD_COUNT 300000

// structure represents thread args (start idx, end idx and partial sum)
typedef struct _args {
    int st_idx;
    int end_idx;
    double sum;
} args;

void free_resources(pthread_t *threads, args *threads_args) {
    free(threads_args);
    free(threads);
}

// thread routine function (calculation of partial sum)
void *calc_sum(void *param) {
    args *arguments = (args*) param;
    double sum = 0;

    for (int i = arguments->st_idx; i <= arguments->end_idx; i++) {
        sum += 1.0/(i*4.0 + 1.0);
        sum -= 1.0/(i*4.0 + 3.0);
    }
    // write result in the structure's field
    arguments->sum = sum;

    return CORRECT_EXIT_CODE;
}
// check string on not digit characters consistence
int has_no_digit(char* str)  {
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
    if (has_no_digit(str)) {
        printf("invalid arg: %s\n", str);
        // finish the process by exit() function
        exit(EXCEPTION_EXIT_CODE);    
    }
    // check 0 input arg value
    size_t arg_len = strlen(str);
    if (arg_len == 1) {
        if (str[0] == '0') {
            return 0;
        }
    }
    // strtol() returns 0 if parse int was failed
    long int in_arg_value = strtol(str, NULL, 10);
    if (!in_arg_value || in_arg_value == LONG_MIN || in_arg_value == LONG_MAX) {
        printf("invalid arg: %s\n", str);
        // finish the process by exit() function
        exit(EXCEPTION_EXIT_CODE);
    }
    return (int) in_arg_value;
}
// check correctness of thread count value
int check_thread_count(int thread_count, int iteration_count) {
    // not less then one thread should executing in programm
    if (thread_count < 1) {
        printf("invalid thread count: %d, should be at less 1\n", thread_count);
        return EXCEPTION_EXIT_CODE;
    }
    // check thread count
    if (thread_count > iteration_count) {
        printf("invalid thread count: %d, should be less then iteration count: %d\n", thread_count, iteration_count);
        return EXCEPTION_EXIT_CODE;
    }
    // check MAX_THREAD_COUNT excess:
    if (thread_count >= MAX_THREAD_COUNT) {
        printf("invalid thread count: %d, should be less then MAX_THREAD_COUNT: %d\n", thread_count, MAX_THREAD_COUNT);
        return EXCEPTION_EXIT_CODE;        
    }
    return CORRECT_EXIT_CODE;
}
// check argc value function
int check_argc(int argc) {
    // check argc value
    if (argc != INPUT_ARGS_COUNT + 1) {
        // print exception message
        printf("invalid arg count: %d, should be 2\n", argc - 1);
        // finish the process by returning from main
        return EXCEPTION_EXIT_CODE;
    }
    return CORRECT_EXIT_CODE;
}
// init thread_args array: args[thread_count]
void init_thread_args(args* thread_args, int thread_count, int iteration_count) {
    int iter_block_size = iteration_count / thread_count;

    for (int i = 0; i < thread_count; i++) {
        thread_args[i].st_idx = i * iter_block_size;
        thread_args[i].end_idx = thread_args[i].st_idx + iter_block_size - 1;
    }
    // init end_idx for last thread (last thread is a 'main' thread)
    int last_thread_idx = thread_count - 1;
    thread_args[last_thread_idx].end_idx = iteration_count - 1;
}
// create threads with start routine 
int start_all_threads(pthread_t *thread_id_array, args* thread_args, int last_thread_idx) {
    // create child threads with start routin:
    for (int i = 0; i < last_thread_idx; i++) {
        int exc_code = pthread_create(&thread_id_array[i], NULL, calc_sum, &thread_args[i]);
        // check exception code
        if (exc_code) {
                char *error_message = strerror(exc_code);
                // print exception message which match to errno value
                fprintf(stderr, "%s%d%s%s\n", "can't create thread ", i, ": ", error_message);

                // finish the process by returning from main
                return EXCEPTION_EXIT_CODE;                
        }
    }
    return CORRECT_EXIT_CODE;
}
// joining all threads and collecting partial sum (including partial sum on parent thread)
int join_threads(double *pi, pthread_t* thread_id_array, args* thread_args, int last_thread_idx) {
    // join child threads and collect partial sum:
    for (int i = 0; i < last_thread_idx; i++) {
        int exc_code = pthread_join(thread_id_array[i], NULL);
        // check exception code
        if (exc_code) {
            char *error_message = strerror(exc_code);
            fprintf(stderr, "%s%d%s%s\n", "can't join thread ", i, ": ", error_message);

            return EXCEPTION_EXIT_CODE;
        }
        // increment pi on calculated partial sum
        *pi += thread_args[i].sum;
    }
    // calc partial sum on parent thread
    calc_sum(&thread_args[last_thread_idx]);
    *pi += thread_args[last_thread_idx].sum;
    *pi *= 4;

    return CORRECT_EXIT_CODE;
}

int main(int argc, char** argv) {
    // check argc value:
    if (check_argc(argc) == EXCEPTION_EXIT_CODE) {
        return EXCEPTION_EXIT_CODE;
    }
    // count of creating threads
    int thread_count = try_cast_arg_to_int(argv[1]);
    // count of iterations
    int iteration_count = try_cast_arg_to_int(argv[2]);

    // check thread count:
    if (check_thread_count(thread_count, iteration_count) == EXCEPTION_EXIT_CODE) {
        return EXCEPTION_EXIT_CODE;
    }
    // idx of the last thread
    int last_thread_idx = thread_count - 1;

    // allocate dynamic memory for array of thread args (in parent thread):
    args *thread_args = (args*) malloc(sizeof(args)*thread_count);
    // check malloc execution correctness
    if (thread_args == NULL) {
        perror("can't allocate memory for thread_args");
        return EXCEPTION_EXIT_CODE;
    }
    // init args for each thread:
    init_thread_args(thread_args, thread_count, iteration_count);
    
    // allocate dynamic memory for array of thread id's (pthread_t array):
    pthread_t *thread_id_array = (pthread_t*) malloc(sizeof(pthread_t) * (last_thread_idx));   // we need to remember threads indices to join() with them
    // check malloc execution correctness
    if (thread_id_array == NULL) {
        perror("can't allocate memory for thread_id_array");
        // free already allocated resources
        free(thread_args);
        return EXCEPTION_EXIT_CODE;
    }
    // create child threads with start routin:
    if (start_all_threads(thread_id_array, thread_args, last_thread_idx) == EXCEPTION_EXIT_CODE) {
        free_resources(thread_id_array, thread_args);

        return EXCEPTION_EXIT_CODE;
    }
    double pi = 0;
    // join child threads and collect partial sum:
    if (join_threads(&pi, thread_id_array, thread_args, last_thread_idx) == EXCEPTION_EXIT_CODE) {
        free_resources(thread_id_array, thread_args);

        return EXCEPTION_EXIT_CODE;
    }
    // print the result
    printf("pi calc value - %.15g iteration count: %d, thread count: %d \n", pi, iteration_count, thread_count);

    // free resources (in parent thread)
    free_resources(thread_id_array, thread_args);

    return CORRECT_EXIT_CODE;
}

#undef CORRECT_EXIT_CODE
#undef EXCEPTION_EXIT_CODE
#undef INPUT_ARGS_COUNT
#undef MAX_THREAD_COUNT
