#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include "tools.h"
#include "constants_define.h"

void str_to_int(int a, char *buffer) {
    int cur_idx = 0;

    if (a == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while(a > 0) {
        int cur_digit = a % 10;
        buffer[cur_idx] = (char) cur_digit + '0';
        
        cur_idx++;
        a /= 10;
    }
    int value_size = cur_idx;
    // reverse buffer:
    for (int i = 0; i < value_size / 2; i++) {
        int copy = buffer[i];
        int end_idx = value_size - 1 - i;

        buffer[i] = buffer[end_idx];
        buffer[end_idx] = copy;
    }
    buffer[value_size] = '\0';
}

void set_thread_name(char* thread_name_buffer, int thread_idx) {
    char *part1 = "Thread";
    char part2[MAX_INT_STR_SIZE];

    str_to_int(thread_idx, part2);
    strcpy(thread_name_buffer, part1);
    strcat(thread_name_buffer, part2);
}

// reading input strings:
int read_strings(args *arg_arr) {
    int string_count = 0;

    while (string_count < MAX_STRING_COUNT) {
        int cur_idx = string_count;
        char *buffer = arg_arr[cur_idx].string;
        char *thread_name = arg_arr[cur_idx].thread_name;
        int *time_delay = &arg_arr[cur_idx].delay;

        char *result_ptr = fgets(buffer, MAX_STRING_SIZE, stdin);

        if (result_ptr == NULL) { // error accured/EOF and no bytes were entered
              //check reading error
            int error_status = ferror(stdin);
            if (error_status != SUCCESS) {
                fprintf(stderr, "an error occurred while reading from stdin\n");

                return FAILED;
            }
            // check EOF
            int eof_status = feof(stdin);
            if (eof_status != SUCCESS) {
                // no data were transfered to buffer so just break from reading loop
                printf("\nEOF is reached, the reading is over\n");
                break;  
            }
        }
        // check just '\n' was entered (some data were transfered to buffer)
        if (buffer[0] == '\n') {
            // the reading is over
            break;
        }
        int string_size = (int) strlen(buffer);

        // reset '\n' character on '\0' if it is exists
        if (buffer[string_size - 1] == '\n') {
            buffer[string_size - 1] = '\0';
        }
        // init remaining args:
        *time_delay = DELTA_TIME * string_size;
        set_thread_name(thread_name, cur_idx);
        string_count++;
    }

    return string_count;
}

void print_args_info() {
    printf("possible arg values:\n");
    printf("arg='1': log adding items to the list.\n");
    printf("arg='0': don't log adding items to the list.\n");
}

// check string on not digit characters consistence
int has_not_digit(char* str)  {
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
    if (has_not_digit(str)) {
        printf("invalid arg: %s\n", str);
        exit(FAILED);
    }
    // check 0 input arg value
    size_t arg_len = strlen(str);
    if (arg_len == 1) {
        if (str[0] == '0') {
            return 0;
        }
    }
    // strtol() returns 0 if parse int was failed
    long int in_arg_value = strtol(str, NULL, BASE);
    if (in_arg_value == CAST_INT_FAILED || in_arg_value == LONG_MIN || in_arg_value == LONG_MAX) {
        printf("invalid arg: %s\n", str);
        exit(FAILED);
    }
    return (int) in_arg_value;
}

int parse_log_state(int argc, char *argv[]) {
    if (argc != ARGS_COUNT) {
        printf("%s%d%s\n","invalid args count - ", argc - 1, " should be 1:");
        print_args_info();

        return FAILED;
    }
    char *arg_str = argv[ARG_IDX];
    int arg = try_cast_arg_to_int(arg_str);

    if (arg == LOG_ENABLE || arg == LOG_DISABLE) {
        return arg;
    }
    printf("%s%d\n", "invalid log_flag value: ", arg);
    print_args_info();

    return FAILED;
}

#include "constants_undef.h"
