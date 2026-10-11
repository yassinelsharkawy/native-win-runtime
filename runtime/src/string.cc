#include <cstddef>

extern "C" size_t wcslen(const wchar_t* str)
{
    const wchar_t* s = str;

    while (*s)
        ++s;

    return (s - str);
}
