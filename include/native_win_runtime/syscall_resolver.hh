/*
 * native-win-runtime
 * Dynamic runtime syscall resolution for Windows x64
 */

#pragma once

#include <cstdint>

namespace nwr
{
    struct ResolvedSyscall
    {
        uint32_t number;
        uintptr_t syscall_ret;
        bool valid;

        constexpr explicit operator bool() const noexcept { return valid; }
    };

    enum class ResolveError : uint32_t
    {
        Success = 0,
        NtdllNotFound,
        FunctionNotFound,
        InvalidStubAddress,
        UnrecognizedStubPattern,
        SyscallRetNotFound,
    };

    struct ResolveResult
    {
        ResolvedSyscall syscall;
        ResolveError error;

        constexpr explicit operator bool() const noexcept { return error == ResolveError::Success; }
    };

    [[nodiscard]] const char* resolve_error_string(ResolveError error) noexcept;

    [[nodiscard]] ResolveResult ResolveSyscall(const char* function_name) noexcept;
    [[nodiscard]] ResolveResult ResolveSyscall(const wchar_t* function_name) noexcept;

    [[nodiscard]] uintptr_t GetNtdllBase() noexcept;

    template <typename Prototype>
    auto Invoke(const ResolvedSyscall& syscall, auto... args);
}

namespace nt
{
    using ResolvedSyscall = nwr::ResolvedSyscall;
    using ResolveError = nwr::ResolveError;
    using ResolveResult = nwr::ResolveResult;

    using nwr::resolve_error_string;
    using nwr::ResolveSyscall;
    using nwr::GetNtdllBase;
    using nwr::Invoke;
}
