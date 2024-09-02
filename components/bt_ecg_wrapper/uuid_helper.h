#ifndef UUID_HELPER
#define UUID_HELPER

#include <array>
#include <cstdint>
#include <iomanip>
#include <stdexcept>
#include <string_view>

namespace uuid {

using uuid128_t = std::array<std::uint8_t, 16>;

namespace internal {

    constexpr std::uint8_t hex_char_to_byte(char c)
    {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        } else {
            return 0xFF;
        }
    }

    constexpr std::uint8_t hex_pair_to_byte(char high, char low)
    {
        return (hex_char_to_byte(high) << 4) | hex_char_to_byte(low);
    }

} // internal

/**
 * @brief Converts a UUID string to a byte array.
 *
 * This function takes a UUID string in the standard 8-4-4-4-12 format and converts it
 * into a 16-byte array. It skips over any dashes in the UUID string.
 *
 * @param uuid_str A string view representing the UUID in the standard format
 *                 (e.g., "16695c8c-ef83-4e58-9688-020537829756").
 * @return A 16-element std::array of uint8_t representing the UUID as bytes.
 *         If the input string contains invalid characters, an empty array is returned.
 *
 * @note The function assumes that the input string is correctly formatted.
 */
constexpr uuid128_t to_byte_array(std::string_view uuid_str)
{
    uuid128_t byte_array = {};
    std::size_t index = 0;

    for (std::size_t i = 0; i < uuid_str.size(); ++i) {
        if (uuid_str[i] == '-') {
            continue;
        }
        if (i + 1 < uuid_str.size()) {
            std::uint8_t byte = internal::hex_pair_to_byte(uuid_str[i], uuid_str[i + 1]);
            if (byte == 0xFF) {
                return {};
            }
            byte_array[index++] = byte;
            ++i;
        }
    }

    return byte_array;
}

/**
 * @brief Converts a 16-byte array representing a UUID to a string.
 *
 * This function takes a 16-byte array and converts it to a UUID string in the standard
 * 8-4-4-4-12 format.
 *
 * @param uuid_bytes A 16-element std::array of uint8_t representing the UUID.
 * @return A std::string representing the UUID in the standard format
 *         (e.g., "16695c8c-ef83-4e58-9688-020537829756").
 */
std::string to_string(const std::array<std::uint8_t, 16>& uuid_bytes)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < uuid_bytes.size(); ++i) {
        oss << std::setw(2) << static_cast<int>(uuid_bytes[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) {
            oss << '-';
        }
    }

    return oss.str();
}

/**
 * @brief Converts a 16-byte array representing a UUID to a string.
 *
 * This function takes a 16-byte array and converts it to a UUID string in the standard
 * 8-4-4-4-12 format.
 *
 * @param uuid_bytes A 16-element std::array of uint8_t representing the UUID.
 * @return A std::string representing the UUID in the standard format
 *         (e.g., "16695c8c-ef83-4e58-9688-020537829756").
 */
std::string to_string(const std::uint8_t* uuid_bytes)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < 16; ++i) {
        oss << std::setw(2) << static_cast<int>(uuid_bytes[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) {
            oss << '-';
        }
    }

    return oss.str();
}

}

#endif // UUID_HELPER
