#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define STRING_COUNT 3
#define THREAD_COUNT 4
#define MAX_STRING_SIZE 100

#define SUCCESS 0
#define FAILED 1

typedef struct _args {
  char (*strings)[MAX_STRING_SIZE];
  int string_count;
} args;

void *print_strings(void* arg) {
  args* thread_args = (args*) arg;
  int string_count = thread_args->string_count;

  for (int i = 0; i < string_count; i++) {
    printf("%s", thread_args->strings[i]);
  }
}

// creating thread with print string sequence routine and handling errors
int try_create_thread(pthread_t *thread, int thread_idx, args *arg) {
  int code = pthread_create(thread, NULL, print_strings, (void*) arg);

  if (code != SUCCESS) {
    char* error_message = strerror(code);
    fprintf(stderr, "%s%s%s%s\n", "can't create thred ", thread_idx, ": ", error_message);

    return FAILED;
  }
  return SUCCESS;
}

// joining the thread with exception handling-
int try_join_thread(pthread_t *thread, int thread_idx) {
  int code = pthread_join(*thread, NULL);
  
  if (code != SUCCESS) {
      char* error_message = strerror(code);
      fprintf(stderr, "%s%d%s%s\n", "can't join thread ", thread_idx, ": ", error_message);

      return FAILED;
  }
  return SUCCESS;
}

//  starting thread's routines
int create_threads(pthread_t *threads, args* thread_args) {
    for (int i = 0; i < THREAD_COUNT; i++) {
      int code = try_create_thread(&threads[i], i, &thread_args[i]);

      if (code == FAILED) {return FAILED;}
    }
    return SUCCESS;
}

// joining all threads
int join_threads(pthread_t *threads) {
  for (int i = 0; i < THREAD_COUNT; i++) {
    int code = try_join_thread(&threads[i], i);
    
    if (code == FAILED) {return FAILED;}
  }
  return SUCCESS;
}

// generating string sequence and initialising thread argument with it
void init_thread_args(args *thread_args, int thread_idx, char (*thread_strings)[MAX_STRING_SIZE]) {
  char *string_token = "str ";
  int token_size = 4;
  thread_args->string_count = STRING_COUNT;
  thread_args->strings = thread_strings;

  for (int string_idx = 0; string_idx < STRING_COUNT; string_idx++) {
    // init string tokens by number of threads:
    for (int token_idx = 0; token_idx <= thread_idx; token_idx++) {
    
      // copy token data:
      for (int i = 0; i < token_size; i++) {
        thread_args->strings[string_idx][token_idx * token_size + i] = string_token[i];
      }
      int string_last_idx = (thread_idx + 1) * token_size;
      // paste '\n' and '\0' characters at the end of the string

      thread_args->strings[string_idx][string_last_idx] = '\n';
      thread_args->strings[string_idx][string_last_idx + 1] = '\0';
    }
  }
}

// init all threads' args:
void init_args(args* threads_args, char (*strings)[STRING_COUNT][MAX_STRING_SIZE]) {
  for (int thread_idx = 0; thread_idx < THREAD_COUNT; thread_idx++) {
    init_thread_args(&threads_args[thread_idx], thread_idx, strings[thread_idx]);
  }
}

int main() {
  pthread_t threads[THREAD_COUNT];
  args threads_args[THREAD_COUNT];
  char threads_strings[THREAD_COUNT][STRING_COUNT][MAX_STRING_SIZE];

  init_args(threads_args, threads_strings);

  int create_status = create_threads(threads, threads_args);
  if (create_status == FAILED) {return FAILED;}

  int join_status = join_threads(threads);
  if (join_status == FAILED) {return FAILED;}

  return SUCCESS;
}

#undef STRING_COUNT
#undef SUCCESS
#undef FAILED
#undef THREAD_COUNT
#undef MAX_STRING_SIZE
