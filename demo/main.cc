/*
 * native-win-runtime — Demonstration
 *
 * Resolves NT syscall numbers at runtime, then performs a safe
 * read-only file operation using direct syscall dispatch.
 */

#include <phnt_windows.h>
#include <phnt.h>
#include <ntpebteb.h>
#include <native_win_runtime.hh>
#include <cstdio>

static constexpr const wchar_t* DEFAULT_FILE = L"\\??\\C:\\Windows\\System32\\notepad.exe";
static constexpr ULONG DEFAULT_BYTES = 64;
static constexpr const char* VERSION_STRING = "1.0.0";

static void print_banner()
{
    printf("\n");
    printf("  native-win-runtime demonstration\n");
    printf("  Windows x64 direct NT syscall dispatch\n");
    printf("  Version %s\n", VERSION_STRING);
    printf("\n");
    printf("  Built without the C runtime (CRT-free)\n");
    printf("  Syscall numbers resolved at runtime from ntdll.dll\n");
    printf("\n");
}

static void print_separator()
{
    printf("  ----------------------------------------\n");
}

static void print_ok(const char* msg)
{
    printf("  [OK]   %s\n", msg);
}

static void print_fail(const char* msg, int32_t status)
{
    nwr::Status s(status);
    printf("  [FAIL] %s (NTSTATUS: 0x%08lX %s)\n", msg, static_cast<uint32_t>(status), s.to_string());
}

static void print_fail_resolve(const char* func, nwr::ResolveError err)
{
    printf("  [FAIL] Resolve %s: %s\n", func, nwr::resolve_error_string(err));
}

static void print_hex_dump(const uint8_t* data, ULONG len)
{
    printf("\n  Hex dump (%lu bytes):\n  ", len);
    for (ULONG i = 0; i < len; ++i)
    {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0 && i + 1 < len)
            printf("\n  ");
    }
    printf("\n");
}

static void print_help()
{
    printf("Usage: nwr-demo [OPTIONS]\n\n");
    printf("Options:\n");
    printf("  --file <path>    NT path to read (default: notepad.exe)\n");
    printf("  --bytes <count>  Number of bytes to read (default: 64, max: 4096)\n");
    printf("  --help           Show this help message\n");
    printf("  --version        Show version information\n");
    printf("\nExamples:\n");
    printf("  nwr-demo\n");
    printf("  nwr-demo --file \"\\??\\C:\\Windows\\System32\\cmd.exe\" --bytes 128\n");
}

static bool wstr_equal(const wchar_t* a, const wchar_t* b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return false;
        ++a;
        ++b;
    }
    return *a == *b;
}

static const wchar_t* skip_token(const wchar_t* p)
{
    while (*p == L' ' || *p == L'\t')
        ++p;
    if (*p == L'\0')
        return p;

    if (*p == L'"')
    {
        ++p;
        while (*p && *p != L'"')
            ++p;
        if (*p == L'"')
            ++p;
    }
    else
    {
        while (*p && *p != L' ' && *p != L'\t')
            ++p;
    }
    return p;
}

static const wchar_t* get_next_token(const wchar_t*& p, wchar_t* token_buf, int max_len)
{
    while (*p == L' ' || *p == L'\t')
        ++p;
    if (*p == L'\0')
        return nullptr;

    int idx = 0;
    if (*p == L'"')
    {
        ++p;
        while (*p && *p != L'"' && idx < max_len - 1)
        {
            token_buf[idx++] = *p++;
        }
        if (*p == L'"')
            ++p;
    }
    else
    {
        while (*p && *p != L' ' && *p != L'\t' && idx < max_len - 1)
        {
            token_buf[idx++] = *p++;
        }
    }
    token_buf[idx] = L'\0';
    return token_buf;
}

struct DemoConfig
{
    const wchar_t* file_path;
    wchar_t custom_path[512];
    ULONG byte_count;
    bool show_help;
    bool show_version;
};

static DemoConfig parse_command_line()
{
    DemoConfig config{};
    config.file_path = DEFAULT_FILE;
    config.byte_count = DEFAULT_BYTES;
    config.show_help = false;
    config.show_version = false;

    const PEB* peb = NtCurrentPeb();
    if (!peb || !peb->ProcessParameters)
        return config;

    const wchar_t* cmd = peb->ProcessParameters->CommandLine.Buffer;
    if (!cmd)
        return config;

    cmd = skip_token(cmd);

    wchar_t token[512];
    while (get_next_token(cmd, token, 512))
    {
        if (wstr_equal(token, L"--help") || wstr_equal(token, L"-h"))
        {
            config.show_help = true;
            return config;
        }
        else if (wstr_equal(token, L"--version") || wstr_equal(token, L"-v"))
        {
            config.show_version = true;
            return config;
        }
        else if (wstr_equal(token, L"--file"))
        {
            if (get_next_token(cmd, config.custom_path, 512))
            {
                config.file_path = config.custom_path;
            }
        }
        else if (wstr_equal(token, L"--bytes"))
        {
            wchar_t count_str[32];
            if (get_next_token(cmd, count_str, 32))
            {
                int count = 0;
                for (int i = 0; count_str[i] >= L'0' && count_str[i] <= L'9'; ++i)
                {
                    count = count * 10 + (count_str[i] - L'0');
                }
                if (count > 0 && count <= 4096)
                    config.byte_count = static_cast<ULONG>(count);
            }
        }
    }

    return config;
}

__declspec(code_seg(".text$start")) extern "C" int start()
{
    DemoConfig config = parse_command_line();

    if (config.show_help)
    {
        print_help();
        return 0;
    }

    if (config.show_version)
    {
        printf("native-win-runtime demo v%s\n", VERSION_STRING);
        return 0;
    }

    print_banner();
    print_separator();

    // Step 1: Locate ntdll.dll
    uintptr_t ntdll_base = nwr::GetNtdllBase();
    if (!ntdll_base)
    {
        print_fail("Locate ntdll.dll", 0);
        return 1;
    }
    printf("  [OK]   Located ntdll.dll at %p\n", reinterpret_cast<void*>(ntdll_base));

    // Step 2: Resolve NtOpenFile
    auto open_result = nwr::ResolveSyscall("NtOpenFile");
    if (!open_result)
    {
        print_fail_resolve("NtOpenFile", open_result.error);
        return 1;
    }
    printf("  [OK]   Resolved NtOpenFile, syscall number: 0x%04X\n", open_result.syscall.number);

    // Step 3: Resolve NtReadFile
    auto read_result = nwr::ResolveSyscall("NtReadFile");
    if (!read_result)
    {
        print_fail_resolve("NtReadFile", read_result.error);
        return 1;
    }
    printf("  [OK]   Resolved NtReadFile, syscall number: 0x%04X\n", read_result.syscall.number);

    // Step 4: Resolve NtClose
    auto close_result = nwr::ResolveSyscall("NtClose");
    if (!close_result)
    {
        print_fail_resolve("NtClose", close_result.error);
        return 1;
    }
    printf("  [OK]   Resolved NtClose, syscall number: 0x%04X\n", close_result.syscall.number);

    print_separator();

    // Step 5: Open the file
    HANDLE file_handle = nullptr;
    UNICODE_STRING file_path_us;
    OBJECT_ATTRIBUTES obj_attrs;
    IO_STATUS_BLOCK io_status;

    RtlInitUnicodeString(&file_path_us, config.file_path);
    InitializeObjectAttributes(&obj_attrs, &file_path_us, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

    NTSTATUS status = nwr::Invoke<decltype(&NtOpenFile)>(
        open_result.syscall,
        &file_handle,
        FILE_GENERIC_READ | SYNCHRONIZE,
        &obj_attrs,
        &io_status,
        FILE_SHARE_READ,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(status))
    {
        print_fail("Open file", status);
        return 1;
    }
    printf("  [OK]   Opened file (handle: %p)\n", file_handle);

    // Step 6: Read bytes
    uint8_t buffer[4096];
    ULONG bytes_to_read = config.byte_count;
    if (bytes_to_read > sizeof(buffer))
        bytes_to_read = sizeof(buffer);

    status = nwr::Invoke<decltype(&NtReadFile)>(
        read_result.syscall,
        file_handle,
        nullptr,
        nullptr,
        nullptr,
        &io_status,
        buffer,
        bytes_to_read,
        nullptr,
        nullptr);

    if (!NT_SUCCESS(status))
    {
        print_fail("Read file", status);
        nwr::Invoke<decltype(&NtClose)>(close_result.syscall, file_handle);
        return 1;
    }

    ULONG bytes_read = static_cast<ULONG>(io_status.Information);
    printf("  [OK]   Read %lu bytes\n", bytes_read);

    // Step 7: Close the handle
    status = nwr::Invoke<decltype(&NtClose)>(close_result.syscall, file_handle);
    if (!NT_SUCCESS(status))
    {
        print_fail("Close file", status);
        return 1;
    }
    print_ok("Closed file handle");

    // Step 8: Display results
    print_separator();
    print_hex_dump(buffer, bytes_read);

    printf("\n  All operations completed successfully.\n\n");

    return 0;
}
