#include "arkoi_fs_ext/cursor.h"

#include <string.h>

static uint16_t cursor_read_le16_raw(const uint8_t* data) {
    return (uint16_t)(data[0]) | (uint16_t)(data[1] << 8U);
}

static uint32_t cursor_read_le32_raw(const uint8_t* data) {
    return (uint32_t)(data[0]) | (uint32_t)(data[1] << 8U) | (uint32_t)(data[2] << 16U) | (uint32_t)(data[3] << 24U);
}

void cursor_read_u8(cursor* cursor, uint8_t* destination) {
    *destination = cursor->data[0];
    cursor->data += 1U;
}

void cursor_read_u16(cursor* cursor, uint16_t* destination) {
    *destination = cursor_read_le16_raw(cursor->data);
    cursor->data += 2U;
}

void cursor_read_u32(cursor* cursor, uint32_t* destination) {
    *destination = cursor_read_le32_raw(cursor->data);
    cursor->data += 4U;
}

void cursor_read_bytes(cursor* cursor, uint8_t* destination, const size_t count) {
    memcpy(destination, cursor->data, count);
    cursor->data += count;
}

void cursor_read_chars(cursor* cursor, char* destination, const size_t count) {
    for (size_t index = 0U; index < count; ++index) {
        destination[index] = (char) cursor->data[index];
    }
    cursor->data += count;
}