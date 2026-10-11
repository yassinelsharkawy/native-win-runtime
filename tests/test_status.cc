/*
 * native-win-runtime — Tests: Status wrapper
 */

#include <native_win_runtime/status.hh>
#include <cstdio>

static int g_test_failures = 0;

static void check(bool condition, const char* desc)
{
    if (condition)
    {
        printf("  [PASS] %s\n", desc);
    }
    else
    {
        printf("  [FAIL] %s\n", desc);
        ++g_test_failures;
    }
}

void test_status()
{
    printf("\n--- Status tests ---\n");

    nwr::Status success(0x00000000);
    check(success.is_success(), "STATUS_SUCCESS is success");
    check(!success.is_error(), "STATUS_SUCCESS is not error");
    check(static_cast<bool>(success), "STATUS_SUCCESS converts to true");
    check(success.severity() == 0, "STATUS_SUCCESS severity == 0");

    nwr::Status info(0x40000003);
    check(info.is_info(), "STATUS_IMAGE_NOT_AT_BASE is info");
    check(info.is_success(), "Info status is still success");
    check(!info.is_error(), "Info status is not error");

    nwr::Status warning(static_cast<int32_t>(0x80000005));
    check(warning.is_warning(), "STATUS_BUFFER_OVERFLOW is warning");
    check(!warning.is_success(), "Warning is not success");
    check(!warning.is_error(), "Warning is not error");

    nwr::Status error(static_cast<int32_t>(0xC0000005));
    check(error.is_error(), "STATUS_ACCESS_VIOLATION is error");
    check(!error.is_success(), "Error is not success");
    check(error.severity() == 3, "Error severity == 3");

    nwr::Status s1(0x00000000);
    nwr::Status s2(0x00000000);
    nwr::Status s3(static_cast<int32_t>(0xC0000001));
    check(s1 == s2, "Equality: same values are equal");
    check(s1 != s3, "Inequality: different values are not equal");

    check(success.code() == 0, "STATUS_SUCCESS code == 0");
    check(error.code() == 5, "STATUS_ACCESS_VIOLATION code == 5");

    const char* name = success.to_string();
    check(name[0] == 'S', "STATUS_SUCCESS to_string starts with S");

    const char* err_name = error.to_string();
    check(err_name[0] == 'S', "STATUS_ACCESS_VIOLATION to_string starts with S");
}
