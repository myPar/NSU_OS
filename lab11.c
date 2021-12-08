#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define CORRECT_EXIT_CODE 0
#define EXCEPTION_EXIT_CODE 1

// structure represents thread args
typedef struct _args {
        char *string;
        int iteration_number;
        int flag;  // specified parent or child thread use this arg (0 - parent, 1 - child)
} args;

// parent args
static args parent_args;
// child args
static args child_args;
// global flag represents which thread should print string next ('0' - parent, '1' - child)
static int global_print_flag = 0; // first is parent thread
// global mutex
static pthread_mutex_t mutex;

int reset_flag(int flag) {
        if (flag == 0) {
                return 1;
        }
        else if (flag == 1) {
                return 0;
        }
        else {
                // invalid branch
                assert(0);
        }
}

// print strings function
void *print_strings(void *param) {
        args *arguments = (args*) param;
        // get arg structure fields:
        int iteration_idx = 0;
        int max_iteration_count = arguments->iteration_number;
        int thread_flag = arguments->flag;
        char *string = arguments->string;

        while (iteration_idx < max_iteration_count) {
                // try to lock mutex
                if (pthread_mutex_lock(&mutex)) {
                        if (thread_flag == 0) {
                                perror("Parent thread can't lock the mutex");
                                return EXCEPTION_EXIT_CODE;
                        }
                        else {
                                perror("Child thread can't lock the mutex");
                                return EXCEPTION_EXIT_CODE;
                        }
                }
                // chekc print order, the global flag should be equal to current thread flag
                if (global_print_flag != thread_flag) {
                        //try to unlock mutex and continue
                        if (pthread_mutex_unlock(&mutex)) {
                                if (thread_flag == 0) {
                                        perror("Parent thread can't unlock the mutex");
                                        return EXCEPTION_EXIT_CODE;
                                }
                                else {
                                        perror("Child thread can't unlock the mutex");
                                        return EXCEPTION_EXIT_CODE;
                                }
                        }
                        continue;
                }
                // print string
                printf("%s\n", string);
                // increment iteration idx
                iteration_idx++;
                // reset flag
                global_print_flag = reset_flag(global_print_flag);
                // try unlock mutex
                if (pthread_mutex_unlock(&mutex)) {
                        if (thread_flag == 0) {
                                perror("Parent thread can't unlock the mutex");
                                return EXCEPTION_EXIT_CODE;
                        }
                        else {
                                perror("Child thread can't unlock the mutex");
                                return EXCEPTION_EXIT_CODE;
                        }
                }
        }
        return CORRECT_EXIT_CODE;
}
  
int main() {
        // create child thread and execute print_strings function
        pthread_t thread_id;
        // init default mutex
        pthread_mutex_init(&mutex, NULL);

        // init child thread args:
        child_args.flag = 1;
        child_args.iteration_number = 10;
        child_args.string = "Child";

        // init parent thread args:
        parent_args.flag = 0;
        parent_args.iteration_number = 10;
        parent_args.string = "Parent";

        int exc_code = pthread_create(&thread_id, NULL, print_strings, &child_args);
        // check exception code
        if (exc_code) {
                // print exception message which match to errno value
                perror("can't create a thread");
                // finish the process by returning from main
                return EXCEPTION_EXIT_CODE;
        }
        // call function in parent thread
        int code = print_strings(&parent_args);
        // join child thread and get result
        void *result;

        //printf("\s", "not joined");

        if (pthread_join(thread_id, &result)) {
                perror("can't join Child thread");
                return EXCEPTION_EXIT_CODE;
        }

        //printf("joined\n");

        int result_value = result;
        int code_value = code;
        // check threads execution results
        if (result_value == EXCEPTION_EXIT_CODE || code_value == EXCEPTION_EXIT_CODE) {
                return EXCEPTION_EXIT_CODE;
        }
        //printf("%d %d\n", result_value, code_value);

        // destroy mutex
        if (pthread_mutex_destroy(&mutex)) {
                perror("Can't destroy mutex");
                return EXCEPTION_EXIT_CODE;
        }

        pthread_exit(CORRECT_EXIT_CODE);
}
#undef CORRECT_EXIT_CODE
#undef EXCEPTION_EXIT_CODE
                                