/*
 * native-win-runtime
 * Dynamic runtime syscall number extraction from ntdll.dll stubs
 *
 * Windows x64 ntdll syscall stub layout (standard):
 *
 *   +00  4C 8B D1        mov r10, rcx
 *   +03  B8 xx xx 00 00  mov eax, <syscall_number>
 *   +08  F6 04 25 08 03  test byte ptr [7FFE0308h], 1
 *        FE 7F 01
 *   +10  75 03           jne +3
 *   +12  0F 05           syscall
 *   +14  C3              ret
 *   +15  CD 2E           int 2Eh
 *   +17  C3              ret
 */

#include <native_win_runtime/syscall_resolver.hh>

#include <phnt_windows.h>
#include <phnt.h>
#include <ntpebteb.h>

namespace nwr
{
    const char* resolve_error_string(ResolveError error) noexcept
    {
        switch (error)
        {
        case ResolveError::Success:
            return "Success";
        case ResolveError::NtdllNotFound:
            return "ntdll.dll not found in loaded modules";
        case ResolveError::FunctionNotFound:
            return "Function not found in ntdll.dll exports";
        case ResolveError::InvalidStubAddress:
            return "Exported function address is null or invalid";
        case ResolveError::UnrecognizedStubPattern:
            return "ntdll stub does not match the expected x64 syscall pattern";
        case ResolveError::SyscallRetNotFound:
            return "Could not locate syscall;ret sequence in stub";
        default:
            return "Unknown error";
        }
    }

    static uintptr_t find_ntdll_base() noexcept
    {
        const PEB* peb = NtCurrentPeb();
        if (!peb || !peb->Ldr)
            return 0;

        const LIST_ENTRY* head = &peb->Ldr->InMemoryOrderModuleList;
        const LIST_ENTRY* entry = head->Flink;

        while (entry != head)
        {
            const auto* ldr_entry = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

            if (ldr_entry->BaseDllName.Buffer && ldr_entry->BaseDllName.Length >= 9 * sizeof(WCHAR))
            {
                const wchar_t* name = ldr_entry->BaseDllName.Buffer;
                bool match =
                    (name[0] == L'n' || name[0] == L'N') &&
                    (name[1] == L't' || name[1] == L'T') &&
                    (name[2] == L'd' || name[2] == L'D') &&
                    (name[3] == L'l' || name[3] == L'L') &&
                    (name[4] == L'l' || name[4] == L'L') &&
                    name[5] == L'.' &&
                    (name[6] == L'd' || name[6] == L'D') &&
                    (name[7] == L'l' || name[7] == L'L') &&
                    (name[8] == L'l' || name[8] == L'L');

                if (match)
                    return reinterpret_cast<uintptr_t>(ldr_entry->DllBase);
            }

            entry = entry->Flink;
        }

        return 0;
    }

    static uintptr_t cached_ntdll_base = 0;

    uintptr_t GetNtdllBase() noexcept
    {
        if (cached_ntdll_base == 0)
            cached_ntdll_base = find_ntdll_base();
        return cached_ntdll_base;
    }

    static uintptr_t find_export(uintptr_t module_base, const char* name) noexcept
    {
        if (!module_base || !name)
            return 0;

        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module_base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            return 0;

        const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS64*>(module_base + dos->e_lfanew);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            return 0;

        const auto& export_dir_entry = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (export_dir_entry.VirtualAddress == 0)
            return 0;

        const auto* export_dir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(
            module_base + export_dir_entry.VirtualAddress);

        const auto* names = reinterpret_cast<const uint32_t*>(module_base + export_dir->AddressOfNames);
        const auto* ordinals = reinterpret_cast<const uint16_t*>(module_base + export_dir->AddressOfNameOrdinals);
        const auto* functions = reinterpret_cast<const uint32_t*>(module_base + export_dir->AddressOfFunctions);

        for (uint32_t i = 0; i < export_dir->NumberOfNames; ++i)
        {
            const char* export_name = reinterpret_cast<const char*>(module_base + names[i]);

            const char* a = name;
            const char* b = export_name;
            while (*a && *b && *a == *b)
            {
                ++a;
                ++b;
            }

            if (*a == '\0' && *b == '\0')
            {
                uint16_t ordinal = ordinals[i];
                return module_base + functions[ordinal];
            }
        }

        return 0;
    }

    static bool validate_syscall_stub(const uint8_t* stub, uint32_t& out_number, uintptr_t& out_syscall_ret) noexcept
    {
        // mov r10, rcx  =>  4C 8B D1
        if (stub[0] != 0x4C || stub[1] != 0x8B || stub[2] != 0xD1)
            return false;

        // mov eax, imm32  =>  B8 xx xx xx xx
        if (stub[3] != 0xB8)
            return false;

        out_number =
            static_cast<uint32_t>(stub[4]) |
            (static_cast<uint32_t>(stub[5]) << 8) |
            (static_cast<uint32_t>(stub[6]) << 16) |
            (static_cast<uint32_t>(stub[7]) << 24);

        // Search within the stub for the syscall (0F 05) ; ret (C3) sequence
        for (int offset = 8; offset < 0x20; ++offset)
        {
            if (stub[offset] == 0x0F && stub[offset + 1] == 0x05 && stub[offset + 2] == 0xC3)
            {
                out_syscall_ret = reinterpret_cast<uintptr_t>(&stub[offset]);
                return true;
            }
        }

        return false;
    }

    ResolveResult ResolveSyscall(const char* function_name) noexcept
    {
        ResolveResult result{};
        result.syscall = {};

        uintptr_t ntdll = GetNtdllBase();
        if (!ntdll)
        {
            result.error = ResolveError::NtdllNotFound;
            return result;
        }

        uintptr_t func_addr = find_export(ntdll, function_name);
        if (!func_addr)
        {
            result.error = ResolveError::FunctionNotFound;
            return result;
        }

        const auto* stub = reinterpret_cast<const uint8_t*>(func_addr);
        if (!stub)
        {
            result.error = ResolveError::InvalidStubAddress;
            return result;
        }

        uint32_t number = 0;
        uintptr_t syscall_ret = 0;

        if (!validate_syscall_stub(stub, number, syscall_ret))
        {
            result.error = ResolveError::UnrecognizedStubPattern;
            return result;
        }

        result.syscall.number = number;
        result.syscall.syscall_ret = syscall_ret;
        result.syscall.valid = true;
        result.error = ResolveError::Success;
        return result;
    }

    static int narrow_wchar(const wchar_t* wide, char* narrow, int max_len) noexcept
    {
        int i = 0;
        while (wide[i] && i < max_len - 1)
        {
            if (wide[i] > 127)
                return -1;
            narrow[i] = static_cast<char>(wide[i]);
            ++i;
        }
        narrow[i] = '\0';
        return i;
    }

    ResolveResult ResolveSyscall(const wchar_t* function_name) noexcept
    {
        char narrow[256];
        if (narrow_wchar(function_name, narrow, sizeof(narrow)) < 0)
        {
            ResolveResult result{};
            result.error = ResolveError::FunctionNotFound;
            return result;
        }
        return ResolveSyscall(narrow);
    }
}
