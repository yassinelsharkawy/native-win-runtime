/*
 * native-win-runtime — Demo output helpers
 * Console I/O via NT syscalls for CRT-free builds
 */

#include <phnt_windows.h>
#include <phnt.h>
#include <ntpebteb.h>
#include <cstdio>

extern "C" void putchar_(char c)
{
    IO_STATUS_BLOCK iosb;
    HANDLE consoleHandle = NtCurrentPeb()->ProcessParameters->StandardOutput;

    NtWriteFile(consoleHandle, nullptr, nullptr, nullptr, &iosb, (PVOID)&c, 1, nullptr, nullptr);
}

extern "C" int puts(const char* str)
{
    IO_STATUS_BLOCK iosb;
    HANDLE consoleHandle = NtCurrentPeb()->ProcessParameters->StandardOutput;

    while (*str)
    {
        NtWriteFile(consoleHandle, nullptr, nullptr, nullptr, &iosb, (PVOID)str, 1, nullptr, nullptr);
        str++;
    }

    char nl = '\n';
    NtWriteFile(consoleHandle, nullptr, nullptr, nullptr, &iosb, (PVOID)&nl, 1, nullptr, nullptr);

    return 0;
}
