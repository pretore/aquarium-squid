#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <squid.h>

#include <test/cmocka.h>

static void check_instance_error_on_out_is_null(void **state) {
    squid_error = SQUID_ERROR_NONE;
    assert_false(squid_executor_instance(NULL));
    assert_int_equal(SQUID_EXECUTOR_ERROR_OUT_IS_NULL, squid_error);
    squid_error = SQUID_ERROR_NONE;
}

static void check_instance(void **state) {
    squid_error = SQUID_ERROR_NONE;
    struct squid_executor *executor;
    assert_true(squid_executor_instance(&executor));
    squid_error = SQUID_ERROR_NONE;
}

int main(int argc, char *argv[]) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(check_instance_error_on_out_is_null),
            cmocka_unit_test(check_instance),
    };
    //cmocka_set_message_output(CM_OUTPUT_XML);
    return cmocka_run_group_tests(tests, NULL, NULL);
}
