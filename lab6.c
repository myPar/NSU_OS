#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STRING_SIZE 100
#define DELTA_TIME 100000
#define MAX_STRING_COUNT 100
#define SUCCESS 0
#define FAILED -1

// arg of each thread routine
typedef struct _args {
    char string[MAX_STRING_SIZE];
    int delay;  //in microseconds
} args;

void *print_string_with_delay(void *arg) {
  args* routine_arg = (args*) arg;
  int delay = routine_arg->delay;
  char *string = routine_arg->string;

  usleep(delay);
  printf("%s\n", string);
}

int read_strings(args *arg_arr) {
  int string_count = 0;

  while (string_count < MAX_STRING_COUNT) {
      int cur_idx = string_count;
      char *buffer = arg_arr[cur_idx].string;
      int *time_delay = &arg_arr[cur_idx].delay;

      if (fgets(buffer, MAX_STRING_SIZE, stdin) == NULL) {
          //check reading error
          if (ferror(stdin)) {    // TODO: fix
            fprintf(stderr, "an error occurred while reading from stdin\n");

            return FAILED;
          }
          // check EOF
          if (feof(stdin)) {   // TODO: fix
            printf("\nEOF is reached, the reading is over\n");
            break;
          }
      }
      // check just '\n' was entered
      if (buffer[0] == '\n') {
        // the reading is over
        break;
      }
      int string_size = (int) strlen(buffer);

      // reset '\n' character on '\0' if it is exists
      if (buffer[string_size - 1] == '\n') {
        buffer[string_size - 1] = '\0';
      }
      // init time delay arg
      *time_delay = DELTA_TIME * string_size;
      string_count++;
  }

  return string_count;
}

int allocate_threads_space(int string_count, pthread_t **threads_space) {
  *threads_space = (pthread_t*) malloc(sizeof(pthread_t) * string_count);

  if (*threads_space == NULL) {
      // errno is setted
      perror("can't allocate memory for pthread_t structures");
      return FAILED;
  }
  return SUCCESS;
}

// creating a thread with exception handling
int try_create_thread(pthread_t *thread, args* thread_arg, int thread_idx) {
  int code = pthread_create(thread, NULL, print_string_with_delay, thread_arg);

  if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%d%s%s\n", "can't create a thread ", thread_idx, ":", error_message);

        return FAILED;
  }
  return SUCCESS;
}

//  starting thread's routines
int create_threads(int string_count, pthread_t *threads, args *thread_args) {
    for (int i = 0; i < string_count; i++) {
      if (try_create_thread(&threads[i], &thread_args[i], i) == FAILED) {return FAILED;}
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

// joining all threads
int join_threads(int string_count, pthread_t *threads) {
  for (int i = 0; i < string_count; i++) {
    if (try_join_thread(&threads[i], i) == FAILED) {return FAILED;}
  }
  return SUCCESS;
}

int main() {
  int string_count = 0;
  args arg_arr[MAX_STRING_COUNT];
  pthread_t *threads_space;

  printf("enter the string sequence:\n");
  string_count = read_strings(arg_arr);

  if (string_count == FAILED) {return FAILED;}
  if (string_count == 0) {
    printf("Nothing to sort: no strings were entered.\n");

    return SUCCESS;
  }
  printf("%s%d%s","The reading is over. Totally read strings ", string_count, "\n");
  
  // allocate thread's space
  int allocate_status = allocate_threads_space(string_count, &threads_space);
  if (allocate_status == FAILED) {return FAILED;}

  printf("\nstart sorting...\n");

  // create threads
  int create_status = create_threads(string_count, threads_space, arg_arr);
  if (create_status == FAILED) {
    free(threads_space);
    return FAILED;
  }

  // joining all threads
  int join_status = join_threads(string_count, threads_space);
  if (join_status == FAILED) {
    free(threads_space);
    return FAILED;
  }
  free(threads_space);

  printf("\nthe sorting is completed successfully.\n");

  return SUCCESS;
}

#undef MAX_STRING_SIZE
#undef DELTA_TIME
#undef MAX_STRING_COUNT
#undef SUCCESS
#undef FAILED
