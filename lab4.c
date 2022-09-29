#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SUCCESS 0
#define FAILED 1
#define WAIT_TIME 2

// infinity routine of printing text
void *print_text(void *arg) {
  char *text = (char*) arg;

  while (1) {
    printf("%s\n", text);
    pthread_testcancel();
  }
}

// cancel a thread with exception handling
int try_cancel_thread(pthread_t *thread) {
  int code = pthread_cancel(*thread);

  if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%s\n", "can't cancel a child thread: ", error_message);

        return FAILED;
  }
  return SUCCESS;
}

// joining the thread with exception handling-
int try_join_thread(pthread_t *thread) {
  int code = pthread_join(*thread, NULL);

  if (code != SUCCESS) {
      char* error_message = strerror(code);
      fprintf(stderr, "%s%s\n", "can't join child thread: ", error_message);

      return FAILED;
  }
  return SUCCESS;
}

// creating a thread with exception handling
int try_create_thread(pthread_t *thread, void *arg) {
  int code = pthread_create(thread, NULL, print_text, arg);

  if (code != SUCCESS) {
        char* error_message = strerror(code);
        fprintf(stderr, "%s%s\n", "can't create a child thread:", error_message);

        return FAILED;
  }
  return SUCCESS;
}

int main() {
  pthread_t child_thread;
  char* text = "Child thread text\n"; // this text will be printed by child thread

  int create_status = try_create_thread(&child_thread, text);
  if (create_status == FAILED) {return FAILED;}

  sleep(WAIT_TIME);

  int cancel_status = try_cancel_thread(&child_thread);
  if (cancel_status == FAILED) {return FAILED;}

  // join child thread to reliaze the resources:
  int join_status = try_join_thread(&child_thread);
  if (join_status == FAILED) {return FAILED;}

  return SUCCESS;
}

#undef SUCCESS
#undef FAILED
#undef WAIT_TIME
