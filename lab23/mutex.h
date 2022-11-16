#pragma once
#include <pthread.h>

int init_mutex(pthread_mutex_t *mutex);
void print_destroy_error_message(int code);
void print_lock_error_message(int code);
void print_unlock_error_message(int code);
void print_destroy_error_message(int code);