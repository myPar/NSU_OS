#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SUCCESS 0
#define FAILED 1
#define WAIT_TIME 2
#define PRINT_DELTA_TIME 1 * 100000 // in microseconds

char* printing_text = "Child thread text\n"; // this text will be printed by child thread
char* termination_text = "Child thread is terminated\n";

void cleanup_handler(void* arg) {
  printf(termination_text);
}

// infinity routine of printing text
void *print_text() {
  pthread_cleanup_push(cleanup_handler, NULL);  // push cleanup routine to the stack

  while (1) {
    pthread_testcancel();
    printf("%s\n", printing_text);
    usleep(PRINT_DELTA_TIME);
  }
  pthread_cleanup_pop(1);

  return NULL;
}

// joining the thread with exception handling:
int try_join_thread(pthread_t *thread) {
  int code = pthread_join(*thread, NULL);

  if (code != SUCCESS) {
      char* error_message = strerror(code);
      fprintf(stderr, "%s%s\n", "can't join child thread: ", error_message);

      return FAILED;
  }
  return SUCCESS;
}

// cancel a thread with exception handling
int try_cancel_thread(pthread_t *thread) {
  int code = pthread_cancel(*thread);

  if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%s\n", "can't cancel a child thread:", error_message);

        return FAILED;
  }
  return SUCCESS;
}

// creating a thread with exception handling
int try_create_thread(pthread_t *thread) {
  int code = pthread_create(thread, NULL, print_text, NULL);

  if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%s\n", "can't create a child thread:", error_message);

        return FAILED;
  }
  return SUCCESS;
}

int main() {
  pthread_t child_thread;

  if (try_create_thread(&child_thread) == FAILED) {return FAILED;}

  sleep(WAIT_TIME);

  if (try_cancel_thread(&child_thread) == FAILED) {return FAILED;}

  // block main thread to prevent process killing before cancelation cleanup handler will be executed
  if (try_join_thread(&child_thread) == FAILED) {return FAILED;}

  printf("the execution is succefully completed\n");

  return SUCCESS;
}

#undef SUCCESS
#undef FAILED
#undef WAIT_TIME
#undef PRINT_DELTA_TIME
