#include <pthread.h>
#include <stdio.h>
#include <errno.h>

#define CORRECT_EXIT_CODE 0
#define EXCEPTION_EXIT_CODE 1

// structure represents thread args
typedef struct _args {
        char *string;
        int iteration_number;
} args;

// print strings function
void *print_strings(void *param) {
        args *arguments = (args*) param;

        for (int i = 0; i < arguments->iteration_number; i++) {
                printf("%s\n", arguments->string);
        }
        return CORRECT_EXIT_CODE;
}

// parent args
static args parent_args = {"Parent", 10};
// child args
static args child_args = {"Child", 10};

int main() {
        // create child thread and execute print_strings function
        pthread_t thread_id;
        int exc_code = pthread_create(&thread_id, NULL, print_strings, &child_args);
        // check exception code
        if (exc_code) {
                // print exception message which match to errno value
                perror("can't create a thread");
                // finish the process by returning from main
                return EXCEPTION_EXIT_CODE;
        }
        // call function in parent thread
        print_strings(&parent_args);

        pthread_exit(CORRECT_EXIT_CODE);
}
#undef CORRECT_EXIT_CODE
#undef EXCEPTION_EXIT_CODE

