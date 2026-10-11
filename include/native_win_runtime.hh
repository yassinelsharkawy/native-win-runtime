/*
 * native-win-runtime
 * Windows x64 Native API & CRT-Free Runtime Framework
 */

#pragma once

#include <cstdint>
#include <native_win_runtime/status.hh>
#include <native_win_runtime/syscall_resolver.hh>

extern "C" uint64_t NtSyscallBridge(uint32_t syscallNumber, uintptr_t syscallRetGadget, ...);

namespace nwr
{
    constexpr uint32_t SYSCALL_MAX_ARGS = 16;

    template <typename T> struct NtPrototype;

    template <typename Ret, typename... ProtoArgs> struct NtPrototype<Ret (*)(ProtoArgs...)>
    {
        auto operator()(uint32_t syscallNumber, uintptr_t syscallRetGadget, ProtoArgs... args) const -> Ret
        {
            constexpr uint32_t ArgCount = sizeof...(ProtoArgs);

            static_assert(ArgCount <= SYSCALL_MAX_ARGS, "Too many arguments for an NT syscall");

            return (Ret)(NtSyscallBridge(syscallNumber, syscallRetGadget, args...));
        }
    };

    template <typename Prototype> inline constexpr NtPrototype<Prototype> NtCall;

    template <typename Prototype>
    inline auto Invoke(const ResolvedSyscall& syscall, auto... args)
    {
        return NtCall<Prototype>(syscall.number, syscall.syscall_ret, args...);
    }
}

namespace nt
{
    template <typename Prototype> inline constexpr auto& NtCall = nwr::NtCall<Prototype>;
    using nwr::Invoke;
}
