/*
 * native-win-runtime — Tests: Syscall resolver
 */

#include <native_win_runtime.hh>
#include <phnt_windows.h>
#include <phnt.h>
#include <cstdio>

static int g_resolver_failures = 0;

static void check(bool condition, const char* desc)
{
    if (condition)
    {
        printf("  [PASS] %s\n", desc);
    }
    else
    {
        printf("  [FAIL] %s\n", desc);
        ++g_resolver_failures;
    }
}

void test_resolver()
{
    printf("\n--- Resolver tests ---\n");

    uintptr_t base = nwr::GetNtdllBase();
    check(base != 0, "GetNtdllBase returns nonzero");

    auto result = nwr::ResolveSyscall("NtClose");
    check(static_cast<bool>(result), "ResolveSyscall(NtClose) succeeds");
    check(result.syscall.valid, "NtClose: resolved syscall is valid");
    check(result.syscall.syscall_ret != 0, "NtClose: syscall_ret is nonzero");

    auto result2 = nwr::ResolveSyscall("NtOpenFile");
    check(static_cast<bool>(result2), "ResolveSyscall(NtOpenFile) succeeds");

    auto result3 = nwr::ResolveSyscall("NtReadFile");
    check(static_cast<bool>(result3), "ResolveSyscall(NtReadFile) succeeds");

    auto result4 = nwr::ResolveSyscall("NtWriteFile");
    check(static_cast<bool>(result4), "ResolveSyscall(NtWriteFile) succeeds");

    auto bad = nwr::ResolveSyscall("NtThisFunctionDoesNotExist99");
    check(!static_cast<bool>(bad), "ResolveSyscall(nonexistent) fails");
    check(bad.error == nwr::ResolveError::FunctionNotFound, "Nonexistent reports FunctionNotFound");

    auto wresult = nwr::ResolveSyscall(L"NtClose");
    check(static_cast<bool>(wresult), "ResolveSyscall(wchar_t NtClose) succeeds");

    // Verify resolved numbers are consistent between calls
    auto result_again = nwr::ResolveSyscall("NtClose");
    check(result.syscall.number == result_again.syscall.number, "NtClose number is consistent");

    // Verify syscall and Zw are the same
    auto zw_result = nwr::ResolveSyscall("ZwClose");
    check(static_cast<bool>(zw_result), "ResolveSyscall(ZwClose) succeeds");
    check(zw_result.syscall.number == result.syscall.number, "ZwClose and NtClose have same number");

    // Test that the resolved NtClose actually works
    NTSTATUS status = nwr::Invoke<decltype(&NtClose)>(result.syscall, (HANDLE)nullptr);
    nwr::Status s(status);
    check(s.is_error(), "NtClose(nullptr) returns error status");
}

extern void test_status();
extern void test_runtime_helpers();

__declspec(code_seg(".text$start")) extern "C" int start()
{
    printf("\n  native-win-runtime test suite\n\n");

    test_status();
    test_runtime_helpers();
    test_resolver();

    printf("\n  Tests complete.\n\n");
    return 0;
}
