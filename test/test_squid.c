#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <squid.h>

#include <test/cmocka.h>

int main(int argc, char *argv[]) {
    const struct CMUnitTest tests[] = {
    };
    //cmocka_set_message_output(CM_OUTPUT_XML);
    return cmocka_run_group_tests(tests, NULL, NULL);
}
