/* Implements pthread_barrier_* functions using POSIX 1003.1c functions
   since barriers were not part of the standard until 1003.1j was
   folded in POSIX.1-2008 Issue 6 */

#ifndef BARRIER_H_
#define BARRIER_H_

#include <pthread.h>
#include <errno.h>

#ifdef __APPLE__

#ifndef PTHREAD_BARRIER_SERIAL_THREAD
#define PTHREAD_BARRIER_SERIAL_THREAD 2999 /* any unused errno int is ok */
#endif

typedef int pthread_barrierattr_t;
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} pthread_barrier_t;

static int pthread_barrier_init(pthread_barrier_t *barrier,
				const pthread_barrierattr_t *attr,
				unsigned int count) {
    if(count == 0) {
        errno = EINVAL;
        return -1;
    }
    if(pthread_mutex_init(&barrier->mutex, 0) < 0)
        return -1;
    if(pthread_cond_init(&barrier->cond, 0) < 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return -1;
    }
    barrier->tripCount = count;
    barrier->count = 0;

    return 0;
}

static int pthread_barrier_destroy(pthread_barrier_t *barrier) {
    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
    return 0;
}

static int pthread_barrier_wait(pthread_barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);
    ++(barrier->count);
    if(barrier->count >= barrier->tripCount) {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return 1;
    }
    pthread_cond_wait(&barrier->cond, &(barrier->mutex));
    pthread_mutex_unlock(&barrier->mutex);
    return 0;
}

static int pthread_barrierattr_init(pthread_barrierattr_t *attr) { *attr = 0; return 0; }
static int pthread_barrierattr_destroy(pthread_barrierattr_t *attr) { return 0; }

#else
/* nothing assuming pthread_barrier_* exist ... */
#endif /* Apple */

#endif
