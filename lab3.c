#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define STRING_COUNT 3
#define THREAD_COUNT 4

#define SUCCESS 0
#define FAILED 1

void *print_string(void* arg) {
  int thread_number = (int) arg;
  char *string_token = "str ";

  for (int i = 0; i < STRING_COUNT; i++) {
    for (int j = 0; j < thread_number; j++) {
      printf("%s", string_token);
    }
    printf("\n");
  }
}

// creating thread with print string sequence routine and handling errors
int try_create_thread(pthread_t *thread, int thread_idx) {
  int thread_number = thread_idx + 1;
  int code = pthread_create(thread, NULL, print_string, (void*) thread_number);

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
int create_threads(pthread_t *threads) {
    for (int i = 0; i < THREAD_COUNT; i++) {
      int code = try_create_thread(&threads[i], i);

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

int main() {
  pthread_t threads[THREAD_COUNT];
  int create_status = create_threads(threads);
  if (create_status == FAILED) {return FAILED;}

  int join_status = join_threads(threads);
  if (join_status == FAILED) {return FAILED;}

  return SUCCESS;
}

#undef STRING_COUNT
#undef SUCCESS
#undef FAILED
#undef THREAD_COUNT
