#include <pthread.h>
#include "list.h"

#define MAX_STRING_SIZE 100
// thread arg structure
typedef struct _args args;
struct _args {
    char string[MAX_STRING_SIZE];
    int delay; // in microseconds
};
#undef MAX_STRING_SIZE

int read_strings(args *arg_arr);
int allocate_threads_space(int string_count, pthread_t **threads_space);
int destroy_list(LinkedList *list);
int create_list(LinkedList *list, pthread_mutex_t *mutex);
int parse_log_state(int argc, char *argv[]);
