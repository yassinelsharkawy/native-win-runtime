#include <algorithm>
#include <limits>
#include <winternl.h>

extern "C" VOID NTAPI RtlInitUnicodeString(PUNICODE_STRING destination, PCWSTR source)
{
    if (destination == nullptr)
        return;

    if (source == nullptr)
    {
        destination->Length = 0;
        destination->MaximumLength = 0;
        destination->Buffer = nullptr;
        return;
    }

    constexpr auto maxLength = std::numeric_limits<USHORT>::max() - sizeof(WCHAR);

    auto length = std::min(wcslen(source) * sizeof(WCHAR), maxLength);

    destination->Length = static_cast<USHORT>(length);
    destination->MaximumLength = static_cast<USHORT>(length + sizeof(WCHAR));
    destination->Buffer = const_cast<PWSTR>(source);
}
