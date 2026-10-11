/*
 * native-win-runtime
 * Type-safe NTSTATUS wrapper and status classification engine
 */

#pragma once

#include <cstdint>

namespace nwr
{
    class Status
    {
    public:
        using ValueType = int32_t;

        constexpr Status() noexcept : value_(0) {}
        constexpr Status(ValueType value) noexcept : value_(value) {}

        constexpr ValueType value() const noexcept { return value_; }
        constexpr explicit operator ValueType() const noexcept { return value_; }

        constexpr bool is_success() const noexcept { return (value_ >= 0); }
        constexpr bool is_info() const noexcept
        {
            return ((static_cast<uint32_t>(value_) & 0xC0000000) == 0x40000000);
        }
        constexpr bool is_warning() const noexcept
        {
            return ((static_cast<uint32_t>(value_) & 0xC0000000) == 0x80000000);
        }
        constexpr bool is_error() const noexcept
        {
            return ((static_cast<uint32_t>(value_) & 0xC0000000) == 0xC0000000);
        }

        constexpr explicit operator bool() const noexcept { return is_success(); }

        constexpr uint16_t facility() const noexcept
        {
            return static_cast<uint16_t>((static_cast<uint32_t>(value_) >> 16) & 0x0FFF);
        }

        constexpr uint16_t code() const noexcept
        {
            return static_cast<uint16_t>(static_cast<uint32_t>(value_) & 0xFFFF);
        }

        constexpr uint8_t severity() const noexcept
        {
            return static_cast<uint8_t>((static_cast<uint32_t>(value_) >> 30) & 0x03);
        }

        [[nodiscard]] const char* to_string() const noexcept;

        constexpr bool operator==(const Status& other) const noexcept { return value_ == other.value_; }
        constexpr bool operator!=(const Status& other) const noexcept { return value_ != other.value_; }
        constexpr bool operator==(ValueType raw) const noexcept { return value_ == raw; }
        constexpr bool operator!=(ValueType raw) const noexcept { return value_ != raw; }

    private:
        ValueType value_;
    };
}

namespace nt
{
    using Status = nwr::Status;
}
