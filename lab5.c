#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SUCCESS 0
#define FAILED 1
#define WAIT_TIME 2

typedef struct _args {
  char *termination_text;
  char *print_text;
} args;

void cleanup_handler(void* arg) {
  char *termination_text = (char*) arg;

  printf("%s\n", termination_text);
}

// infinity routine of printing text
void *print_text(void* param) {
  args *thread_args = (args*) param;

  char *print_text = thread_args->print_text;
  char *termination_text = thread_args->termination_text;

  pthread_cleanup_push(cleanup_handler, (void*) termination_text);  // push cleanup routine to the stack

  while (1) {
    printf("%s\n", print_text);
    pthread_testcancel();
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
int try_create_thread(pthread_t *thread, args *arg) {
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

  char *print_text = "Child thread text";
  char *termination_text = "Child thread is terminated";

  args arg = {termination_text, print_text};

  // try to create child thread:
  int create_status = try_create_thread(&child_thread, &arg);
  if (create_status == FAILED) {return FAILED;}

  sleep(WAIT_TIME);

  // try to cancel child thread
  int cancel_status = try_cancel_thread(&child_thread);
  if (cancel_status == FAILED) {return FAILED;}

  // block main thread to prevent process killing before cancelation cleanup handler will be executed
  int join_status = try_join_thread(&child_thread);
  if (join_status == FAILED) {return FAILED;}

  return SUCCESS;
}

#undef SUCCESS
#undef FAILED
#undef WAIT_TIME
