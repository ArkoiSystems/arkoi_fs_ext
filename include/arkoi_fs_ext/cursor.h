#ifndef ARKOI_FS_EXT_CURSOR_H
#define ARKOI_FS_EXT_CURSOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cursor {
    const uint8_t* data;
} cursor;

void cursor_read_u8(cursor* cursor, uint8_t* destination);

void cursor_read_u16(cursor* cursor, uint16_t* destination);

void cursor_read_u32(cursor* cursor, uint32_t* destination);

void cursor_read_bytes(cursor* cursor, uint8_t* destination, size_t count);

void cursor_read_chars(cursor* cursor, char* destination, size_t count);

#ifdef __cplusplus
}
#endif

#endif