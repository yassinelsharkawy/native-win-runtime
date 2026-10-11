/*
 * native-win-runtime
 * NTSTATUS string formatting and classification
 */

#include <native_win_runtime/status.hh>

namespace nwr
{
    struct StatusEntry
    {
        uint32_t code;
        const char* name;
    };

    static constexpr StatusEntry STATUS_TABLE[] = {
        {0x00000000, "STATUS_SUCCESS"},
        {0x00000103, "STATUS_NO_MORE_FILES"},
        {0x40000003, "STATUS_IMAGE_NOT_AT_BASE"},
        {0x80000005, "STATUS_BUFFER_OVERFLOW"},
        {0x80000006, "STATUS_NO_MORE_FILES_WARNING"},
        {0x8000001A, "STATUS_NO_MORE_ENTRIES"},
        {0xC0000001, "STATUS_UNSUCCESSFUL"},
        {0xC0000002, "STATUS_NOT_IMPLEMENTED"},
        {0xC0000004, "STATUS_INFO_LENGTH_MISMATCH"},
        {0xC0000005, "STATUS_ACCESS_VIOLATION"},
        {0xC0000008, "STATUS_INVALID_HANDLE"},
        {0xC000000D, "STATUS_INVALID_PARAMETER"},
        {0xC0000011, "STATUS_END_OF_FILE"},
        {0xC0000017, "STATUS_NO_MEMORY"},
        {0xC0000022, "STATUS_ACCESS_DENIED"},
        {0xC0000023, "STATUS_BUFFER_TOO_SMALL"},
        {0xC0000034, "STATUS_OBJECT_NAME_NOT_FOUND"},
        {0xC0000035, "STATUS_OBJECT_NAME_COLLISION"},
        {0xC000007A, "STATUS_PROCEDURE_NOT_FOUND"},
        {0xC00000BB, "STATUS_NOT_SUPPORTED"},
        {0xC0000135, "STATUS_DLL_NOT_FOUND"},
        {0xC0000139, "STATUS_ENTRYPOINT_NOT_FOUND"}
    };

    const char* Status::to_string() const noexcept
    {
        const uint32_t uval = static_cast<uint32_t>(value_);
        for (const auto& entry : STATUS_TABLE)
        {
            if (entry.code == uval)
                return entry.name;
        }

        if (is_success())
            return "STATUS_SUCCESS (Informational/Success)";
        if (is_info())
            return "STATUS_INFO (Informational)";
        if (is_warning())
            return "STATUS_WARNING (Warning)";
        if (is_error())
            return "STATUS_ERROR (Unmapped Error)";

        return "STATUS_UNKNOWN";
    }
}
