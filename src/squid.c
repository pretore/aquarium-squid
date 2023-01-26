#include <stdlib.h>
#include <pthread.h>
#include <seagrass.h>
#include <triggerfish.h>
#include <squid.h>

static struct triggerfish_strong *executor_ref;
static struct squid_executor *executor;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

bool squid_executor_instance(struct squid_executor **const out) {
    if (!executor) {
        squid_error = SQUID_ERROR_LIBRARY_UNINITIALIZED;
        return false;
    }
    if (!out) {
        squid_error = SQUID_EXECUTOR_ERROR_OUT_IS_NULL;
        return false;
    }
    *out = executor;
    return true;
}

__attribute__((constructor))
void squid_on_load(void) {
    if (executor_ref) {
        return;
    }
    seagrass_required_true(!pthread_mutex_lock(&mutex));
    if (!executor_ref) {
        seagrass_required_true(squid_executor_of(&executor_ref));
        seagrass_required_true(triggerfish_strong_instance(
                executor_ref, (void **) &executor));
    }
    seagrass_required_true(!pthread_mutex_unlock(&mutex));
}

__attribute__((destructor))
void squid_on_unload(void) {
    if (!executor_ref) {
        return;
    }
    seagrass_required_true(!pthread_mutex_lock(&mutex));
    if (executor_ref) {
        seagrass_required_true(triggerfish_strong_release(executor_ref));
        executor_ref = NULL;
    }
    seagrass_required_true(!pthread_mutex_unlock(&mutex));
}
