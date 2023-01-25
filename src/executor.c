#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <seagrass.h>
#include <squid.h>

#include "private/executer.h"
#include "private/future.h"

#ifdef TEST
#include <test/cmocka.h>
#endif

static void invalidate(struct squid_executor *const object) {
    assert(object);
    int error;
    if ((error = pthread_mutex_destroy(&object->threads.mutex))) {
        seagrass_required_true(error == EINVAL);
    }
    if ((error = pthread_cond_destroy(&object->threads.condition))) {
        seagrass_required_true(error == EINVAL);
    }
    if (!triggerfish_weak_destroy(object->self)) {
        seagrass_required_true(TRIGGERFISH_WEAK_ERROR_OBJECT_IS_NULL
                               == triggerfish_error);
    }
    seagrass_required_true(lionfish_concurrent_linked_queue_sr_invalidate(
            &object->tasks));
    *object = (struct squid_executor) {0};
}

bool squid_executor_init(struct squid_executor *const object) {
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL;
        return false;
    }
    *object = (struct squid_executor) {0};
    int error;
    if ((error = pthread_mutex_init(&object->threads.mutex, NULL))) {
        seagrass_required_true(ENOMEM == error);
        squid_error = SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if ((error = pthread_cond_init(&object->threads.condition, NULL))) {
        seagrass_required_true(ENOMEM == error);
        invalidate(object);
        squid_error = SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if (!lionfish_concurrent_linked_queue_sr_init(&object->tasks, 8)) {
        seagrass_required_true(
                LIONFISH_CONCURRENT_LINKED_QUEUE_SR_ERROR_MEMORY_ALLOCATION_FAILED
                == lionfish_error);
        invalidate(object);
        squid_error = SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    atomic_store(&object->is_running, true);
    return true;
}

bool squid_executor_invalidate(struct squid_executor *const object) {
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (atomic_load(&object->is_running)) {
        squid_error = SQUID_EXECUTOR_ERROR_IS_RUNNING;
        return false;
    }
    invalidate(object);
    return true;
}

bool squid_executor_shutdown(struct squid_executor *const object) {
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL;
        return false;
    }
    bool expected = true;
    const bool desired = false;
    if (!atomic_compare_exchange_strong(&object->is_running, &expected,
                                        desired)) {
        squid_error = SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN;
        return false;
    }
    return true;
}

static void on_destroy(void *const object) {
    if (!squid_executor_shutdown(object)) {
        seagrass_required_true(SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN
                               == squid_error);
    }
    uintmax_t count;
    do {
        seagrass_required_true(squid_executor_count(object, &count));
        if (!count) {
            break;
        }
        const struct timespec delay = {
                .tv_nsec = 100000000 /* 100 milliseconds */
        };
        seagrass_required_true(!nanosleep(&delay, NULL)
                               || errno == EINTR);
    } while (true);
    seagrass_required_true(squid_executor_invalidate(object));
}

bool squid_executor_of(struct triggerfish_strong **const out) {
    if (!out) {
        squid_error = SQUID_EXECUTOR_ERROR_OUT_IS_NULL;
        return false;
    }
    struct squid_executor *object = malloc(sizeof(*object));
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if (!squid_executor_init(object)) {
        seagrass_required_true(
                SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED
                == squid_error);
        return false;
    }
    struct triggerfish_strong *strong;
    if (!triggerfish_strong_of(object, on_destroy, &strong)) {
        seagrass_required_true(
                TRIGGERFISH_STRONG_ERROR_MEMORY_ALLOCATION_FAILED
                == triggerfish_error);
        free(object);
        squid_error = SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if (!triggerfish_weak_of(strong, &object->self)) {
        seagrass_required_true(
                TRIGGERFISH_WEAK_ERROR_MEMORY_ALLOCATION_FAILED
                == triggerfish_error);
        seagrass_required_true(triggerfish_strong_release(strong));
        return false;
    }
    *out = strong;
    return true;
}

bool squid_executor_count(const struct squid_executor *const object,
                          uintmax_t *const out) {
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        squid_error = SQUID_EXECUTOR_ERROR_OUT_IS_NULL;
        return false;
    }
    *out = atomic_load(&object->threads.count);
    return true;
}

bool squid_executor_ready(const struct squid_executor *const object,
                          uintmax_t *const out) {
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        squid_error = SQUID_EXECUTOR_ERROR_OUT_IS_NULL;
        return false;
    }
    *out = atomic_load(&object->threads.ready);
    return true;
}

bool squid_executor_is_running(const struct squid_executor *const object,
                               bool *const out) {
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        squid_error = SQUID_EXECUTOR_ERROR_OUT_IS_NULL;
        return false;
    }
    *out = atomic_load(&object->is_running);
    return true;
}

static _Thread_local struct squid_future *task;

static bool is_cancelled(void) {
    enum squid_future_status status;
    seagrass_required_true(squid_future_status(task, &status));
    if (SQUID_FUTURE_STATUS_CANCELLED == status) {
        return true;
    }
    struct squid_executor *executor;
    seagrass_required_true(triggerfish_strong_instance(
            task->executor, (void **) &executor));
    bool is_running;
    seagrass_required_true(squid_executor_is_running(
            executor, &is_running));
    if (!is_running) {
        atomic_store(&task->status, SQUID_FUTURE_STATUS_CANCELLED);
        return true;
    }
    return false;
}

static void *routine(void *object) {
    seagrass_required(object);
    (void) pthread_detach(pthread_self());
    struct squid_executor *const executor = object;
    struct triggerfish_strong *self;
    if (!triggerfish_weak_strong(executor->self, &self)) {
        seagrass_required_true(TRIGGERFISH_WEAK_ERROR_STRONG_IS_INVALID
                               == triggerfish_error);
        return NULL;
    }
    struct triggerfish_strong *out;
    loop:
    while (lionfish_concurrent_linked_queue_sr_remove(&executor->tasks,
                                                      &out)) {
        seagrass_required_true(!pthread_cond_signal(
                &executor->threads.condition));
        seagrass_required_true(triggerfish_strong_instance(
                out, (void **) &task));
        bool is_running;
        seagrass_required_true(squid_executor_is_running(
                executor, &is_running));
        if (is_running) {
            atomic_store(&task->status, SQUID_FUTURE_STATUS_RUNNING);
            task->function(task->args, is_cancelled, &task->out, &task->error);
            if (SQUID_FUTURE_STATUS_RUNNING == atomic_load(&task->status)) {
                atomic_store(&task->status, SQUID_FUTURE_STATUS_DONE);
            }
        } else {
            atomic_store(&task->status, SQUID_FUTURE_STATUS_CANCELLED);
        }
        seagrass_required_true(!pthread_cond_signal(&task->condition));
        seagrass_required_true(triggerfish_strong_release(out));
    }
    seagrass_required_true(
            LIONFISH_CONCURRENT_LINKED_QUEUE_SR_ERROR_QUEUE_IS_EMPTY
            == lionfish_error);
    seagrass_required_true(!pthread_mutex_lock(&executor->threads.mutex));
    struct timespec tp;
    seagrass_required_true(!clock_gettime(CLOCK_REALTIME, &tp));
    seagrass_required_true((tp.tv_sec += 60) >= 0);
    uintmax_t value;
    seagrass_required_true(seagrass_uintmax_t_add(
            1, atomic_fetch_add(&executor->threads.ready, 1), &value));
    int error;
    const struct triggerfish_strong *peek;
    while (!lionfish_concurrent_linked_queue_sr_peek(&executor->tasks,
                                                     &peek)) {
        seagrass_required_true(
                LIONFISH_CONCURRENT_LINKED_QUEUE_SR_ERROR_QUEUE_IS_EMPTY
                == lionfish_error);
        if (!(error = pthread_cond_timedwait(&executor->threads.condition,
                                             &executor->threads.mutex,
                                             &tp))
            || ETIMEDOUT == error) {
            break;
        }
    }
    seagrass_required_true(!pthread_mutex_unlock(&executor->threads.mutex));
    seagrass_required_true(seagrass_uintmax_t_subtract(
            atomic_fetch_sub(&executor->threads.ready, 1), 1, &value));
    if (!error
        || lionfish_concurrent_linked_queue_sr_peek(&executor->tasks,
                                                    &peek)) {
        goto loop;
    }
    seagrass_required_true(seagrass_uintmax_t_subtract(
            atomic_fetch_sub(&executor->threads.count, 1), 1, &value));
    seagrass_required_true(triggerfish_strong_release(self));
    return NULL;
}

static bool enqueue(struct squid_executor *const object,
                    struct triggerfish_strong *const future) {
    assert(object);
    assert(future);
    if (!atomic_load(&object->threads.ready)) {
        pthread_t thread;
        int error;
        if ((error = pthread_create(&thread, NULL, routine, object))) {
            seagrass_required_true(EAGAIN == error);
            if (!atomic_load(&object->threads.count)) {
                squid_error = SQUID_EXECUTOR_ERROR_THREAD_CREATION_FAILED;
                return false;
            }
        }
        uintmax_t value;
        seagrass_required_true(seagrass_uintmax_t_add(
                1, atomic_fetch_add(&object->threads.count, 1), &value));
    }
    if (!lionfish_concurrent_linked_queue_sr_add(&object->tasks, future)) {
        seagrass_required_true(
                LIONFISH_CONCURRENT_LINKED_QUEUE_SR_ERROR_MEMORY_ALLOCATION_FAILED
                == lionfish_error);
        squid_error = SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    seagrass_required_true(!pthread_cond_signal(&object->threads.condition));
    return true;
}

static bool submit(struct squid_executor *const object,
                   struct triggerfish_strong *const executor,
                   squid_function const function,
                   void *const args,
                   struct triggerfish_strong **const out) {
    assert(object);
    assert(executor);
    assert(function);
    assert(out);
    struct triggerfish_strong *future;
    if (!squid_future_of(executor, function, args, &future)) {
        seagrass_required_true(SQUID_FUTURE_ERROR_MEMORY_ALLOCATION_FAILED
                               == squid_error);
        squid_error = SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if (!enqueue(object, future)) {
        seagrass_required_true(SQUID_EXECUTOR_ERROR_THREAD_CREATION_FAILED
                               == squid_error
                               || SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED
                                  == squid_error);
        seagrass_required_true(triggerfish_strong_release(future));
        return false;
    }
    *out = future;
    return true;
}

bool squid_executor_submit(struct squid_executor *const object,
                           squid_function const function,
                           void *const args,
                           struct triggerfish_strong **const out) {
    if (!object) {
        squid_error = SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!function) {
        squid_error = SQUID_EXECUTOR_ERROR_FUNCTION_IS_NULL;
        return false;
    }
    if (!out) {
        squid_error = SQUID_EXECUTOR_ERROR_OUT_IS_NULL;
        return false;
    }
    bool result;
    seagrass_required_true(squid_executor_is_running(object, &result));
    if (!result) {
        squid_error = SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN;
        return false;
    }
    struct triggerfish_strong *self;
    if (!triggerfish_weak_strong(object->self, &self)) {
        seagrass_required_true(TRIGGERFISH_WEAK_ERROR_STRONG_IS_INVALID
                               == triggerfish_error);
        squid_error = SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN;
        return false;
    }
    if (!(result = submit(object, self, function, args, out))) {
        seagrass_required_true(SQUID_EXECUTOR_ERROR_THREAD_CREATION_FAILED
                               == squid_error
                               || SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED
                                  == squid_error);
    }
    seagrass_required_true(triggerfish_strong_release(self));
    return result;
}

