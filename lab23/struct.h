#pragma once

#define MAX_STRING_SIZE 100
#define MAX_NAME_SIZE 20

// thread arg structure
typedef struct _args args;
struct _args {
    char string[MAX_STRING_SIZE];
    int delay; // in microseconds
    char thread_name[MAX_NAME_SIZE];
};
#undef MAX_NAME_SIZE
