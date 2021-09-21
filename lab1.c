#include <pthread.h>
#include <stdio.h>

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
        return 0;
}

// parent args
static args parent_args = {"Parent", 10};
// child args
static args child_args = {"Child", 10};

int main() {
        // create child thread and execute print_strings function
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, print_strings, &child_args);
        // call function in parent thread
        print_strings(&parent_args);

        pthread_exit(0);
}

