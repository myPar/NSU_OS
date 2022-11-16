#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "tools.h"
#include "thread.h"
#include "constants_define.h"
#include "globals.h"

// thread shared resourses
LinkedList *list;
int log_flag = LOG_DISABLE;

int set_log_state(int argc, char* argv[]) {
    int status = parse_log_state(argc, argv);
    if (status == FAILED) {return FAILED;}
    if (status == LOG_ENABLE) {log_flag = LOG_ENABLE;}

    return SUCCESS;
}

int read_user_input(args *arg_arr, int *string_count) {
    printf("enter the string sequence:\n");
    
    *string_count = read_strings(arg_arr);

    if (*string_count == FAILED) {return FAILED;}
    if (*string_count == 0) {
        printf("Nothing to sort: no strings were entered.\n");

        exit(SUCCESS);
    }
    printf("%s%d%s","The reading is over. Totally read strings ", *string_count, "\n");

    return SUCCESS;
}

int main(int argc, char *argv[]) {
    args arg_arr[MAX_STRING_COUNT];
    pthread_t *threads_space;
    int string_count = 0;

    int code = set_log_state(argc, argv);
    if (code != SUCCESS) {return FAILED;}

    code = read_user_input(arg_arr, &string_count);
    if (code != SUCCESS) {return FAILED;}

    int allocate_status = allocate_threads_space(string_count, &threads_space);
    if (allocate_status == FAILED) {return FAILED;}

    list = create_list(EMPTY_MSG);
    if (list == NULL) {return FAILED;}
    
    printf("\nstart adding items...\n");
    
    int start_status = start_threads(string_count, threads_space, arg_arr);
    if (start_status != SUCCESS) {
        destroy_list(list, EMPTY_MSG);
        return FAILED;
    }
    code = terminate_threads(string_count, threads_space);
    if (code != SUCCESS) {
        destroy_list(list, EMPTY_MSG);
        return FAILED;
    }
    printf("\nlist creating is completed successfully.\n");
    printf("strings in sorted order:\n");
    print_list(list, EMPTY_MSG);

    code = destroy_list(list, EMPTY_MSG);
    if (code == FAILED) {return FAILED;}

    return SUCCESS;
}

#include "constants_undef.h"
