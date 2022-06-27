#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "hedera_format.h"

#define DEF_TEST_FORMAT_TH(name, expected, tinybar) \
    static void test_format_tinybar_##name(void **state) \
    { \
        char *actual = hedera_format_tinybar(tinybar); \
        assert_string_equal(actual, expected); \
    }

#define DEF_TEST_FORMAT_TOK(name, expected, amount, decimals) \
    static void test_format_amount_##name(void **state) \
    { \
        char *actual = hedera_format_amount(amount, decimals); \
        assert_string_equal(actual, expected); \
    }


DEF_TEST_FORMAT_TH(issue_5, "5.09826013", 509826013)
DEF_TEST_FORMAT_TH(issue_5_trailing, "5.09826010", 509826010)
DEF_TEST_FORMAT_TH(whole_1, "1", 100000000)
DEF_TEST_FORMAT_TH(whole_1000, "1000", 100000000000)
DEF_TEST_FORMAT_TH(whole_100_4, "100.0004", 10000040000)
DEF_TEST_FORMAT_TH(one_th,  "0.00000001", 1)
DEF_TEST_FORMAT_TH(large_h, "21482.73284812", 2148273284812)

DEF_TEST_FORMAT_TOK(value, "51.8321", 518321, 4)
DEF_TEST_FORMAT_TOK(value_0, "0", 0, 0)
DEF_TEST_FORMAT_TOK(value_neg_1, "0.000004294967295", -1u, 15)
DEF_TEST_FORMAT_TOK(value_bad_decimals, "", -1u, 30)

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_format_tinybar_issue_5),
        cmocka_unit_test(test_format_tinybar_issue_5_trailing),
        cmocka_unit_test(test_format_tinybar_whole_1),
        cmocka_unit_test(test_format_tinybar_whole_1000),
        cmocka_unit_test(test_format_tinybar_whole_100_4),
        cmocka_unit_test(test_format_tinybar_one_th),
        cmocka_unit_test(test_format_tinybar_large_h),
        cmocka_unit_test(test_format_amount_value),
        cmocka_unit_test(test_format_amount_value_0),
        cmocka_unit_test(test_format_amount_value_neg_1),
        cmocka_unit_test(test_format_amount_value_bad_decimals),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
