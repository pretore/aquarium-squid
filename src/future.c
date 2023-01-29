#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <seagrass.h>
#include <squid.h>

#include "private/future.h"

#ifdef TEST
#include <test/cmocka.h>
#endif

static void invalidate(struct squid_future *const object) {
    assert(object);
    int error;
    if ((error = pthread_mutex_destroy(&object->mutex))) {
        seagrass_required_true(error == EINVAL);
    }
    if ((error = pthread_cond_destroy(&object->condition))) {
        seagrass_required_true(error == EINVAL);
    }
    triggerfish_strong_release(object->out);
    triggerfish_strong_release(object->executor);
    *object = (struct squid_future) {0};
}

bool squid_future_init(struct squid_future *const object,
                       struct triggerfish_strong *const executor,
                       squid_function const function,
                       void *const args) {
    if (!object) {
        squid_error = SQUID_FUTURE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!executor) {
        squid_error = SQUID_FUTURE_ERROR_EXECUTOR_IS_NULL;
        return false;
    }
    if (!function) {
        squid_error = SQUID_FUTURE_ERROR_FUNCTION_IS_NULL;
        return false;
    }
    *object = (struct squid_future) {0};
    int error;
    if ((error = pthread_mutex_init(&object->mutex, NULL))) {
        seagrass_required_true(ENOMEM == error);
        squid_error = SQUID_FUTURE_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if ((error = pthread_cond_init(&object->condition, NULL))) {
        seagrass_required_true(ENOMEM == error);
        invalidate(object);
        squid_error = SQUID_FUTURE_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if (!triggerfish_strong_retain(executor)) {
        seagrass_required_true(TRIGGERFISH_STRONG_ERROR_OBJECT_IS_INVALID
                               == triggerfish_error);
        invalidate(object);
        squid_error = SQUID_FUTURE_ERROR_EXECUTOR_IS_INVALID;
        return false;
    }
    object->executor = executor;
    object->function = function;
    object->args = args;
    return true;
}

bool squid_future_invalidate(struct squid_future *const object) {
    if (!object) {
        squid_error = SQUID_FUTURE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    invalidate(object);
    return true;
}

static void on_destroy(void *const object) {
    seagrass_required_true(squid_future_invalidate(object));
}

bool squid_future_of(struct triggerfish_strong *const executor,
                     squid_function const function,
                     void *const args,
                     struct triggerfish_strong **const out) {
    if (!executor) {
        squid_error = SQUID_FUTURE_ERROR_EXECUTOR_IS_NULL;
        return false;
    }
    if (!function) {
        squid_error = SQUID_FUTURE_ERROR_FUNCTION_IS_NULL;
        return false;
    }
    if (!out) {
        squid_error = SQUID_FUTURE_ERROR_OUT_IS_NULL;
        return false;
    }
    struct squid_future *object = malloc(sizeof(*object));
    if (!object) {
        squid_error = SQUID_FUTURE_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if (!squid_future_init(object, executor, function, args)) {
        seagrass_required_true(SQUID_FUTURE_ERROR_EXECUTOR_IS_INVALID
                               == squid_error);
        free(object);
        return false;
    }
    if (!triggerfish_strong_of(object, on_destroy, &object->self)) {
        seagrass_required_true(
                TRIGGERFISH_STRONG_ERROR_MEMORY_ALLOCATION_FAILED
                == triggerfish_error);
        free(object);
        squid_error = SQUID_FUTURE_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    *out = object->self;
    return true;
}

bool squid_future_status(const struct squid_future *object,
                         enum squid_future_status *out) {
    if (!object) {
        squid_error = SQUID_FUTURE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        squid_error = SQUID_FUTURE_ERROR_OUT_IS_NULL;
        return false;
    }
    *out = atomic_load(&object->status);
    return true;
}

bool squid_future_cancel(struct squid_future *const object) {
    if (!object) {
        squid_error = SQUID_FUTURE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    enum squid_future_status expected = SQUID_FUTURE_STATUS_RUNNING;
    while (!atomic_compare_exchange_strong(&object->status,
                                           (int *) &expected,
                                           SQUID_FUTURE_STATUS_CANCELLED)) {
        if (SQUID_FUTURE_STATUS_DONE == expected) {
            squid_error = SQUID_FUTURE_ERROR_FUTURE_IS_DONE;
            return false;
        }
    }
    return true;
}

bool squid_future_get(struct squid_future *const object,
                      struct triggerfish_strong **const out,
                      uintmax_t *const error) {
    if (!object) {
        squid_error = SQUID_FUTURE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        squid_error = SQUID_FUTURE_ERROR_OUT_IS_NULL;
        return false;
    }
    seagrass_required_true(!pthread_mutex_lock(&object->mutex));
    while (SQUID_FUTURE_STATUS_DONE > atomic_load(&object->status)) {
        seagrass_required_true(!pthread_cond_wait(
                &object->condition, &object->mutex));
    }
    seagrass_required_true(!pthread_cond_signal(&object->condition));
    seagrass_required_true(!pthread_mutex_unlock(&object->mutex));
    const enum squid_future_status status = atomic_load(&object->status);
    if (SQUID_FUTURE_STATUS_CANCELLED == status) {
        squid_error = SQUID_FUTURE_ERROR_FUTURE_IS_CANCELLED;
        return false;
    }
    *out = object->out;
    if (object->out) {
        seagrass_required_true(triggerfish_strong_retain(object->out));
    }
    if (error) {
        *error = object->error;
    }
    return true;
}
