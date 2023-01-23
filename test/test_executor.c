#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <triggerfish.h>
#include <time.h>
#include <unistd.h>
#include <squid.h>

#include "private/executer.h"

#include <test/cmocka.h>

static void check_invalidate_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_invalidate(NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL,
                     squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_invalidate(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object = {};
    assert_true(squid_executor_invalidate(&object));
    squid_error = SQUID_ERROR_NONE;
}

static void check_shutdown_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_shutdown(NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL,
                     squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_shutdown_error_on_is_busy_shutting_down(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object = {};
    assert_false(squid_executor_shutdown(&object));
    assert_int_equal(SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN,
                     squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_shutdown(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object = {
            .is_running = true
    };
    assert_true(squid_executor_shutdown(&object));
    squid_error = SQUID_ERROR_NONE;
}

static void check_init_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_init(NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL,
                     squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_init(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object;
    assert_true(squid_executor_init(&object));
    assert_true(squid_executor_shutdown(&object));
    assert_true(squid_executor_invalidate(&object));
    squid_error = SQUID_ERROR_NONE;
}

static void check_init_error_on_memory_allocation_failed(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object;
    pthread_mutex_init_is_overridden = true;
    will_return(cmocka_test_pthread_mutex_init, ENOMEM);
    assert_false(squid_executor_init(&object));
    assert_int_equal(SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED,
                     squid_error);
    pthread_mutex_init_is_overridden = false;
    squid_error = SQUID_ERROR_NONE;
}

static void check_of_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_of(NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_of(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct triggerfish_strong *out;
    assert_true(squid_executor_of(&out));
    assert_true(triggerfish_strong_release(out));
    squid_error = SQUID_ERROR_NONE;
}

static void check_of_error_on_memory_allocation_failed(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct triggerfish_strong *out;
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = true;
    assert_false(squid_executor_of(&out));
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = false;
    assert_int_equal(SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED,
                     squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_count_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_count(NULL, (void *) 1));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL,
                     squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_count_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_count((void *) 1, NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_count(void **state) {
    srand(time(NULL));
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object = {};
    const uintmax_t check = 1 + (rand() % (UINTMAX_MAX - 1));
    atomic_store(&object.threads.count, check);
    uintmax_t count;
    assert_true(squid_executor_count(&object, &count));
    assert_int_equal(count, check);
    squid_error = SQUID_ERROR_NONE;
}

static void check_ready_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_ready(NULL, (void *) 1));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL,
                     squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_ready_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_ready((void *) 1, NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_ready(void **state) {
    srand(time(NULL));
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object = {};
    const uintmax_t check = 1 + (rand() % (UINTMAX_MAX - 1));
    atomic_store(&object.threads.ready, check);
    uintmax_t ready;
    assert_true(squid_executor_ready(&object, &ready));
    assert_int_equal(ready, check);
    squid_error = SQUID_ERROR_NONE;
}

static void check_submit_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_submit(NULL, (void *) 1,
                                       (void *) 1, (void *) 1));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_submit_error_on_function_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_submit((void *) 1, NULL,
                                       (void *) 1, (void *) 1));
    assert_int_equal(SQUID_EXECUTOR_ERROR_FUNCTION_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_submit_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_submit((void *) 1, (void *) 1,
                                       (void *) 1, NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_submit_error_on_is_busy_shutting_down(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor object = {
            .is_running = false
    };
    assert_false(squid_executor_submit(&object, (void *) 1,
                                       (void *) 1, (void *) 1));
    assert_int_equal(SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static uintmax_t random_value;

static void function(void *const args,
                     bool (*const is_cancelled)(void),
                     struct triggerfish_strong **const out,
                     uintmax_t *const error) {
    srand(time(NULL));
    assert_ptr_equal(args, &random_value);
    assert_false(is_cancelled());
    *error = random_value = rand() % UINTMAX_MAX;
}

static void check_submit(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct triggerfish_strong *instance;
    assert_true(squid_executor_of(&instance));
    struct squid_executor *executor;
    assert_true(triggerfish_strong_instance(instance, (void **) &executor));
    struct triggerfish_strong *out;
    assert_true(squid_executor_submit(executor, function,
                                      &random_value, &out));
    assert_true(triggerfish_strong_release(instance));
    struct squid_future *future;
    assert_true(triggerfish_strong_instance(out, (void **) &future));
    struct {
        struct triggerfish_strong *out;
        uintmax_t error;
    } result;
    assert_true(squid_future_get(future, &result.out, &result.error));
    assert_null(result.out);
    assert_int_equal(result.error, random_value);
    assert_true(triggerfish_strong_release(out));

    for (uintmax_t delay = 65, slept; delay && (slept = sleep(delay));
         delay -= slept);
    squid_error = SQUID_ERROR_NONE;
}

int main(int argc, char *argv[]) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(check_invalidate_error_on_object_is_null),
            cmocka_unit_test(check_invalidate),
            cmocka_unit_test(check_shutdown_error_on_object_is_null),
            cmocka_unit_test(check_shutdown_error_on_is_busy_shutting_down),
            cmocka_unit_test(check_shutdown),
            cmocka_unit_test(check_init_error_on_object_is_null),
            cmocka_unit_test(check_init),
            cmocka_unit_test(check_init_error_on_memory_allocation_failed),
            cmocka_unit_test(check_of_error_on_out_is_null),
            cmocka_unit_test(check_of),
            cmocka_unit_test(check_of_error_on_memory_allocation_failed),
            cmocka_unit_test(check_count_error_on_object_is_null),
            cmocka_unit_test(check_count_error_on_out_is_null),
            cmocka_unit_test(check_count),
            cmocka_unit_test(check_ready_error_on_object_is_null),
            cmocka_unit_test(check_ready_error_on_out_is_null),
            cmocka_unit_test(check_ready),
            cmocka_unit_test(check_submit_error_on_object_is_null),
            cmocka_unit_test(check_submit_error_on_function_is_null),
            cmocka_unit_test(check_submit_error_on_out_is_null),
            cmocka_unit_test(check_submit_error_on_is_busy_shutting_down),
            cmocka_unit_test(check_submit),
    };
    //cmocka_set_message_output(CM_OUTPUT_XML);
    return cmocka_run_group_tests(tests, NULL, NULL);
}
