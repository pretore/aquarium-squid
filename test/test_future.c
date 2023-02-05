#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <squid.h>

#include "private/future.h"

#include <test/cmocka.h>

static void check_invalidate_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_invalidate(NULL));
    assert_int_equal(SQUID_FUTURE_ERROR_OBJECT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_invalidate(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_future object = {};
    assert_true(squid_future_invalidate(&object));
    squid_error = SQUID_ERROR_NONE;
}

static void check_init_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_init(NULL, (void *) 1, (void *) 1, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_OBJECT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_init_error_on_executor_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_init((void *) 1, NULL, (void *) 1, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_EXECUTOR_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_init_error_on_function_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_init((void *) 1, (void *) 1, NULL, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_FUNCTION_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_init_error_on_executor_is_invalid(void **state) {
    squid_error = SQUID_ERROR_NONE;
    const uintmax_t check = 0;
    struct triggerfish_strong *executor = (struct triggerfish_strong *) &check;
    struct squid_future object;
    assert_false(squid_future_init(&object, executor, (void *) 1, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_EXECUTOR_IS_INVALID, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void on_destroy(void *object) {

}

static void check_init(void **state) {
    srand(time(NULL));
    squid_error = SQUID_ERROR_NONE;
    struct triggerfish_strong *executor;
    assert_true(triggerfish_strong_of(malloc(1), on_destroy, &executor));
    void *func = (void *) (rand() % UINTMAX_MAX);
    struct squid_future object;
    assert_true(squid_future_init(&object, executor, func, (void *) 76));
    assert_int_equal(atomic_load(&object.status), SQUID_FUTURE_STATUS_PENDING);
    assert_ptr_equal(object.args, (void *) 76);
    assert_ptr_equal(object.executor, executor);
    assert_ptr_equal(object.function, func);
    assert_null(object.self);
    assert_null(object.out);
    assert_true(triggerfish_strong_release(executor));
    assert_true(squid_future_invalidate(&object));
    squid_error = SQUID_ERROR_NONE;
}

static void check_of_error_on_executor_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_of(NULL, (void *) 1, (void *) 1, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_EXECUTOR_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_of_error_on_function_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_of((void *) 1, NULL, (void *) 1, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_FUNCTION_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_of_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_of((void *) 1, (void *) 1, (void *) 1, NULL));
    assert_int_equal(SQUID_FUTURE_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_of_error_on_executor_is_invalid(void **state) {
    squid_error = SQUID_ERROR_NONE;
    const uintmax_t check = 0;
    struct triggerfish_strong *executor = (struct triggerfish_strong *) &check;
    assert_false(squid_future_of(executor, (void *) 1, (void *) 1, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_EXECUTOR_IS_INVALID, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_of(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct triggerfish_strong *executor;
    assert_true(triggerfish_strong_of(malloc(1), on_destroy, &executor));
    struct triggerfish_strong *object;
    assert_true(squid_future_of(executor, (void *) 1, (void *)1, &object));
    assert_true(triggerfish_strong_release(executor));
    assert_true(triggerfish_strong_release(object));
    squid_error = SQUID_ERROR_NONE;
}

static void check_status_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_status(NULL, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_OBJECT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_status_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_status((void *) 1, NULL));
    assert_int_equal(SQUID_FUTURE_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_status(void **state) {
    srand(time(NULL));
    squid_error = SQUID_ERROR_NONE;
    const enum squid_future_status check
            = rand() % SQUID_FUTURE_STATUS_CANCELLED;
    struct squid_future object = {
            .status = check
    };
    enum squid_future_status status = (-1);
    assert_true(squid_future_status(&object, &status));
    assert_int_equal(status, check);
    squid_error = SQUID_ERROR_NONE;
}

static void check_cancel_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_cancel(NULL, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_OBJECT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_cancel_error_on_future_is_done(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_future object = {
            .status = SQUID_FUTURE_STATUS_DONE
    };
    assert_false(squid_future_cancel(&object, NULL));
    assert_int_equal(SQUID_FUTURE_ERROR_FUTURE_IS_DONE, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_cancel(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_future object = {};
    enum squid_future_status status;
    assert_true(squid_future_status(&object, &status));
    assert_int_equal(status, SQUID_FUTURE_STATUS_PENDING);
    enum squid_future_status out;
    assert_true(squid_future_cancel(&object, &out));
    assert_int_equal(out, SQUID_FUTURE_STATUS_PENDING);
    assert_true(squid_future_status(&object, &status));
    assert_int_equal(status, SQUID_FUTURE_STATUS_CANCELLED);
    squid_error = SQUID_ERROR_NONE;
}

static void check_get_error_on_object_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_get(NULL, (void *) 1, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_OBJECT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_get_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_future_get((void *) 1, NULL, (void *) 1));
    assert_int_equal(SQUID_FUTURE_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_get(void **state) {
    srand(time(NULL));
    squid_error = SQUID_ERROR_NONE;
    struct triggerfish_strong *executor;
    assert_true(triggerfish_strong_of(malloc(1), on_destroy, &executor));
    struct squid_future object;
    assert_true(squid_future_init(&object, executor, (void *) 1, NULL));
    assert_true(triggerfish_strong_release(executor));
    assert_true(triggerfish_strong_of(malloc(1), on_destroy, &object.out));
    atomic_store(&object.status, SQUID_FUTURE_STATUS_DONE);
    object.error = rand() % UINTMAX_MAX;
    uintmax_t count;
    assert_true(triggerfish_strong_count(object.out, &count));
    assert_int_equal(count, 1);
    struct {
        struct triggerfish_strong *out;
        uintmax_t error;
    } result;
    assert_true(squid_future_get(&object, &result.out, &result.error));
    assert_ptr_equal(result.out, object.out);
    assert_int_equal(result.error, object.error);
    assert_true(triggerfish_strong_count(object.out, &count));
    assert_int_equal(count, 2);
    assert_true(triggerfish_strong_release(result.out));
    assert_true(squid_future_invalidate(&object));
    squid_error = SQUID_ERROR_NONE;
}

static void check_get_error_on_future_is_cancelled(void **state) {
    srand(time(NULL));
    squid_error = SQUID_ERROR_NONE;
    struct triggerfish_strong *executor;
    assert_true(triggerfish_strong_of(malloc(1), on_destroy, &executor));
    struct squid_future object;
    assert_true(squid_future_init(&object, executor, (void *) 1, NULL));
    assert_true(triggerfish_strong_release(executor));
    assert_true(triggerfish_strong_of(malloc(1), on_destroy, &object.out));
    assert_true(squid_future_cancel(&object, NULL));
    struct {
        struct triggerfish_strong *out;
        uintmax_t error;
    } result;
    assert_false(squid_future_get(&object, &result.out, &result.error));
    assert_int_equal(SQUID_FUTURE_ERROR_FUTURE_IS_CANCELLED, squid_error);
    assert_true(squid_future_invalidate(&object));
    squid_error = SQUID_ERROR_NONE;
}

int main(int argc, char *argv[]) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(check_invalidate_error_on_object_is_null),
            cmocka_unit_test(check_invalidate),
            cmocka_unit_test(check_init_error_on_object_is_null),
            cmocka_unit_test(check_init_error_on_executor_is_null),
            cmocka_unit_test(check_init_error_on_function_is_null),
            cmocka_unit_test(check_init_error_on_executor_is_invalid),
            cmocka_unit_test(check_init),
            cmocka_unit_test(check_of_error_on_executor_is_null),
            cmocka_unit_test(check_of_error_on_function_is_null),
            cmocka_unit_test(check_of_error_on_out_is_null),
            cmocka_unit_test(check_of_error_on_executor_is_invalid),
            cmocka_unit_test(check_of),
            cmocka_unit_test(check_status_error_on_object_is_null),
            cmocka_unit_test(check_status_error_on_out_is_null),
            cmocka_unit_test(check_status),
            cmocka_unit_test(check_cancel_error_on_object_is_null),
            cmocka_unit_test(check_cancel_error_on_future_is_done),
            cmocka_unit_test(check_cancel),
            cmocka_unit_test(check_get_error_on_object_is_null),
            cmocka_unit_test(check_get_error_on_out_is_null),
            cmocka_unit_test(check_get),
            cmocka_unit_test(check_get_error_on_future_is_cancelled),
    };
    //cmocka_set_message_output(CM_OUTPUT_XML);
    return cmocka_run_group_tests(tests, NULL, NULL);
}
