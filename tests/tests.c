#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_symbol_parsing(void **state);
void test_strip_comments(void **state);
void test_tokenize(void **state);
void test_optimize(void **state);

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_symbol_parsing),
        cmocka_unit_test(test_strip_comments),
		cmocka_unit_test(test_tokenize),
		cmocka_unit_test(test_optimize)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
