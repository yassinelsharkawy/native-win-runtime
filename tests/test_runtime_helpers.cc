/*
 * native-win-runtime — Tests: CRT-free runtime helpers
 */

#include <cstdio>
#include <cstddef>
#include <cstdint>

extern "C" void* memset(void*, int, size_t);
extern "C" void* memcpy(void*, const void*, size_t);
extern "C" int memcmp(const void*, const void*, size_t);
extern "C" void* memmove(void*, const void*, size_t);
extern "C" size_t wcslen(const wchar_t*);

struct UNICODE_STRING_TEST
{
    uint16_t Length;
    uint16_t MaximumLength;
    wchar_t* Buffer;
};

extern "C" void RtlInitUnicodeString(UNICODE_STRING_TEST*, const wchar_t*);

static int g_runtime_failures = 0;

static void check(bool condition, const char* desc)
{
    if (condition)
    {
        printf("  [PASS] %s\n", desc);
    }
    else
    {
        printf("  [FAIL] %s\n", desc);
        ++g_runtime_failures;
    }
}

void test_runtime_helpers()
{
    printf("\n--- Runtime helper tests ---\n");

    // memset
    uint8_t buf[64];
    memset(buf, 0xAB, sizeof(buf));
    bool memset_ok = true;
    for (int i = 0; i < 64; ++i)
    {
        if (buf[i] != 0xAB)
        {
            memset_ok = false;
            break;
        }
    }
    check(memset_ok, "memset fills buffer correctly");

    // memcpy
    uint8_t src[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t dst[16];
    memcpy(dst, src, 16);
    check(memcmp(src, dst, 16) == 0, "memcpy copies correctly");

    // memcmp
    uint8_t a[4] = {1, 2, 3, 4};
    uint8_t b[4] = {1, 2, 3, 4};
    uint8_t c[4] = {1, 2, 3, 5};
    check(memcmp(a, b, 4) == 0, "memcmp: equal buffers return 0");
    check(memcmp(a, c, 4) < 0, "memcmp: lesser buffer returns negative");
    check(memcmp(c, a, 4) > 0, "memcmp: greater buffer returns positive");
    check(memcmp(a, c, 3) == 0, "memcmp: partial equal returns 0");

    // memmove — non-overlapping
    uint8_t mv_src[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    uint8_t mv_dst[8];
    memmove(mv_dst, mv_src, 8);
    check(memcmp(mv_src, mv_dst, 8) == 0, "memmove: non-overlapping copy");

    // memmove — overlapping forward
    uint8_t overlap[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    memmove(overlap + 2, overlap, 4);
    check(overlap[2] == 1 && overlap[3] == 2 && overlap[4] == 3 && overlap[5] == 4,
          "memmove: overlapping forward");

    // memmove — overlapping backward
    uint8_t overlap2[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    memmove(overlap2, overlap2 + 2, 4);
    check(overlap2[0] == 3 && overlap2[1] == 4 && overlap2[2] == 5 && overlap2[3] == 6,
          "memmove: overlapping backward");

    // wcslen
    check(wcslen(L"") == 0, "wcslen: empty string");
    check(wcslen(L"hello") == 5, "wcslen: \"hello\" == 5");
    check(wcslen(L"A") == 1, "wcslen: single char == 1");

    // RtlInitUnicodeString
    UNICODE_STRING_TEST us;
    RtlInitUnicodeString(&us, L"Test");
    check(us.Length == 4 * sizeof(wchar_t), "RtlInitUnicodeString: Length correct");
    check(us.MaximumLength == 5 * sizeof(wchar_t), "RtlInitUnicodeString: MaximumLength correct");
    check(us.Buffer != nullptr, "RtlInitUnicodeString: Buffer not null");

    UNICODE_STRING_TEST us_null;
    RtlInitUnicodeString(&us_null, nullptr);
    check(us_null.Length == 0, "RtlInitUnicodeString: null source Length == 0");
    check(us_null.Buffer == nullptr, "RtlInitUnicodeString: null source Buffer == nullptr");
}
