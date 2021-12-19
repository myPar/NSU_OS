#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <semaphore.h>

#define SUCCESS 0
#define FAILED 1
#define CORRECT_STATUS 0
#define EXCEPTION_STATUS -1

// threads id's
#define PARENT_ID 0
#define CHILD_ID 1
// semaphores count
#define SEM_COUNT 2

const int iteration_count = 10;
// semaphores array
sem_t semaphores[SEM_COUNT];

void *print() {

}

// function inits both semaphores, prints exception messages, returs 
int init_semaphores() {
    int status = sem_init(&semaphores[PARENT_ID], 0, 1);
    if (status == EXCEPTION_STATUS) {

    }
}

int main() {
    // init semaphores:
}
#undef SUCCESS
#undef FAILED
#undef CORRECT_STATUS
#undef EXCEPTION_STATUS