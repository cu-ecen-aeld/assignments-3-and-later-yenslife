#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    // casting data from *void to struct thread_data*
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    // wait to obtain
    int usleep_obtain_status = usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // check if usleep success
    if (usleep_obtain_status != 0) {
        ERROR_LOG("usleep_obtain_status error code: %d", usleep_obtain_status);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // obtain mutex
    int mutex_lock_status = pthread_mutex_lock(thread_func_args->mutex);
    
    // check mutex_lock_status
    if (mutex_lock_status != 0) {
        ERROR_LOG("mutex_lock_status error code: %d", mutex_lock_status);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // wait for release
    int usleep_release_status = usleep(thread_func_args->wait_to_release_ms * 1000);

    // check usleep_release_status
    if (usleep_release_status != 0) {
        ERROR_LOG("usleep_release_status error code: %d", usleep_release_status);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    int mutex_unlock_status = pthread_mutex_unlock(thread_func_args->mutex);
    // check mutex_unlock_status
    if (mutex_unlock_status != 0) {
        ERROR_LOG("mutex_unlock_status error code: %d", mutex_unlock_status);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    
    // debug
    DEBUG_LOG("Done Threadfunc");
    
    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    // allocate memory dynamically for the thread data
    struct thread_data *my_thread_data = (struct thread_data*)malloc(sizeof(struct thread_data*));

    // check the memory allocation 
    if (my_thread_data == NULL) {
        ERROR_LOG("Malloc my_thread_data error");
        return false;
    }

    // set up the struct thread data
    my_thread_data->wait_to_obtain_ms = wait_to_obtain_ms;
    my_thread_data->wait_to_release_ms = wait_to_release_ms;
    my_thread_data->mutex = mutex;

    int pthread_create_status = pthread_create(thread, NULL, threadfunc, my_thread_data);

    if (pthread_create_status != 0) {
        ERROR_LOG("pthread_create_status error code: %d", pthread_create_status);
        return false;
    } 
    
    return true;
}






















