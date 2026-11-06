#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>
#include <stdint.h>


uint8_t *base64_decode(const char *text, size_t text_len, size_t max_bytes, size_t *out_size);

#endif /* BASE64_H */
