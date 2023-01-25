#include <stdlib.h>
#include <seagrass.h>
#include <triggerfish.h>
#include <squid.h>

static struct triggerfish_strong *executor_ref;
static struct squid_executor *executor;

bool squid_executor_instance(struct squid_executor **out) {
    if (!out) {
        squid_error = SQUID_EXECUTOR_ERROR_OUT_IS_NULL;
        return false;
    }
    *out = executor;
    return true;
}

__attribute__((constructor))
static void on_load(void) {
    seagrass_required_true(squid_executor_of(&executor_ref));
    seagrass_required_true(triggerfish_strong_instance(
            executor_ref, (void **) &executor));
}

__attribute__((destructor))
static void on_unload(void) {
    seagrass_required_true(triggerfish_strong_release(executor_ref));
}
