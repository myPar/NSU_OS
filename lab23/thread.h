#pragma once
#include <pthread.h>
#include "struct.h"

int start_threads(int threads_count, pthread_t *threads_space, args *arg_arr);
int terminate_threads(int threads_count, pthread_t *threads_space);
int allocate_threads_space(int string_count, pthread_t **threads_space);
