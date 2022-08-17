#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define STRING_COUNT 3
#define STRING_MAX_SIZE 20

#define SUCCESS 0
#define FAILED 1

typedef struct _args {
  char (*strings)[STRING_MAX_SIZE];
  int string_count;
} args;

void *print_string(void* arg) {
  args* string_sequence = (args*) arg;

  int str_count = string_sequence->string_count;

  for (int i = 0; i < str_count; i++) {
    printf(string_sequence->strings[i]);
  }
  printf("\n");
}

// creating thread with print string sequence routine and handling errors
int try_create_thread(pthread_t *thread, args *thread_args, char *thread_name) {
  int code = pthread_create(thread, NULL, print_string, thread_args);

  if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%s%s%s\n", "can't create ", thread_name, ": ", error_message);

        return FAILED;
  }
  return SUCCESS;
}

// joining the thread with exception handling-
int try_join_thread(pthread_t *thread, char *thread_name) {
  int code = pthread_join(*thread, NULL);

  if (code != SUCCESS) {
      char* error_message = strerror(code);
      fprintf(stderr, "%s%s%s%s\n", "can't join ", thread_name, ": ", error_message);

      return FAILED;
  }
  return SUCCESS;
}

char sequence1[STRING_COUNT][STRING_MAX_SIZE] = {"string1\n", "string1\n", "string1\n"};
char sequence2[STRING_COUNT][STRING_MAX_SIZE] = {"string2\n", "string2\n", "string2\n"};
char sequence3[STRING_COUNT][STRING_MAX_SIZE] = {"string3\n", "string3\n", "string3\n"};
char sequence4[STRING_COUNT][STRING_MAX_SIZE] = {"string4\n", "string4\n", "string4\n"};

int main() {
  pthread_t thread1;
  pthread_t thread2;
  pthread_t thread3;
  pthread_t thread4;

  args arg1 = {sequence1, STRING_COUNT};
  args arg2 = {sequence2, STRING_COUNT};
  args arg3 = {sequence3, STRING_COUNT};
  args arg4 = {sequence4, STRING_COUNT};

  // main thread create a child thread and blocks untill child thread completes it's routine:
  if (try_create_thread(&thread1, &arg1, "thread1") == FAILED) {return FAILED;}
  if (try_join_thread(&thread1, "thread1") == FAILED) {return FAILED;}

  if (try_create_thread(&thread2, &arg2, "thread2") == FAILED) {return FAILED;}
  if (try_join_thread(&thread2, "thread2") == FAILED) {return FAILED;}

  if (try_create_thread(&thread3, &arg3, "thread3") == FAILED) {return FAILED;}
  if (try_join_thread(&thread3, "thread3") == FAILED) {return FAILED;}

  if (try_create_thread(&thread4, &arg4, "thread4") == FAILED) {return FAILED;}
  if (try_join_thread(&thread4, "thread4") == FAILED) {return FAILED;}

  return SUCCESS;
}

#undef STRING_COUNT
#undef STRING_MAX_SIZE
#undef SUCCESS
#undef FAILED
