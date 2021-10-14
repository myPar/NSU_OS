#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

// defining constants
#define CORRECT_EXIT_CODE 0 
#define EXCEPTION_EXIT_CODE 1   
#define INPUT_ARGS_COUNT 2
// define free resources macro
#define FREE_RESOURCES      \
    free(thread_args);      \
    free(thread_id_array);

// structure represents thread args (start idx, end idx and partial sum)
typedef struct _args {
    int st_idx;
    int end_idx;
    double sum;
} args;

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

// check cast of string to int (terminate the process if fails)
int check_str_to_int_cast(char* str) {
    long int in_arg_value = strtol(str, NULL, 10);
    if (!in_arg_value || in_arg_value == LLONG_MIN || in_arg_value == LLONG_MAX) {
        printf("invalid arg: %s\n", str);
        // finish the process by exit() function
        exit(EXCEPTION_EXIT_CODE);
    }
    return (int) in_arg_value;
}

int main(int argc, char** argv) {
    // check argc value
    if (argc != INPUT_ARGS_COUNT + 1) {
        // print exception message
        printf("invalid arg count: %d, should be 1\n", argc - 1);
        // finish the process by returning from main
        return EXCEPTION_EXIT_CODE;
    }
    // count of creating threads
    int thread_count = check_str_to_int_cast(argv[1]);
    // count of iterations
    int interation_count = check_str_to_int_cast(argv[2]);

    // not less then one thread should executing in programm
    if (thread_count < 1) {
        printf("invalid arg value: %d, should be more then 1\n", thread_count);
        return EXCEPTION_EXIT_CODE;
    }

    // check thread count
    if (thread_count > interation_count) {
        printf("invalid thread count: %d, should be less then iteration count: %d\n", thread_count, interation_count);
        return EXCEPTION_EXIT_CODE;
    }

    // allocate dynamic memory for array of thread args (in parent thread)
    args *thread_args = (args*) malloc(sizeof(args)*thread_count);
    // check malloc execution correctness
    if (thread_args == NULL) {
        perror("can't allocate memory for thread_args");
        return EXCEPTION_EXIT_CODE;
    }

    // init args for each thread:
    int iter_block_size = interation_count / thread_count;

    for (int i = 0; i < thread_count; i++) {
        thread_args[i].st_idx = i * iter_block_size;
        thread_args[i].end_idx = thread_args[i].st_idx + iter_block_size - 1;
    }
    // init end_idx for last thread (last thread is a 'main' thread)
    int last_thread_idx = thread_count - 1;
    thread_args[last_thread_idx].end_idx = interation_count - 1;
    
    // create child threads with start routin:
    pthread_t *thread_id_array = (pthread_t*) malloc(sizeof(pthread_t) * (last_thread_idx));   // we need to remember threads indices to join() with them
    // check malloc execution correctness
    if (thread_id_array == NULL) {
        perror("can't allocate memory for thread_id_array");
        // free already allocated resources
        free(thread_args);

        return EXCEPTION_EXIT_CODE;
    }

    for (int i = 0; i < last_thread_idx; i++) {
        int exc_code = pthread_create(&thread_id_array[i], NULL, calc_sum, &thread_args[i]);
        // check exception code
        if (exc_code) {
                // print exception message which match to errno value
                perror("can't create a thread");
                // free resources
                FREE_RESOURCES
                // finish the process by returning from main
                return EXCEPTION_EXIT_CODE;                
        }
    }
    double pi = 0;
    // join child threads:
    for (int i = 0; i < last_thread_idx; i++) {
        int exc_code = pthread_join(thread_id_array[i], NULL);
        // check exception code
        if (exc_code) {
            perror("can't join a thread");
            FREE_RESOURCES
            return EXCEPTION_EXIT_CODE;
        }
        // increment pi on calculated partial sum
        pi += thread_args[i].sum;
    }
    // calc partial sum on parent thread
    calc_sum(&thread_args[last_thread_idx]);
    pi += thread_args[last_thread_idx].sum;
    pi *= 4;
    // print the result
    printf("pi calc value - %.15g iteration count: %d, thread count: %d \n", pi, interation_count, thread_count);

    // free resources (in parent thread)
    FREE_RESOURCES
    pthread_exit(CORRECT_EXIT_CODE);
}

#undef CORRECT_EXIT_CODE
#undef EXCEPTION_EXIT_CODE
#undef FREE_RESOURCES
#undef INPUT_ARGS_COUNT