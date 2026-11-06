#ifndef MD5_H
#define MD5_H

#include <stddef.h>
#include <stdint.h>


#define MD5_DIGEST_LEN 16
#define MD5_HEX_LEN 33 // 32 characters plus the terminator

void md5(const void *data, size_t len, unsigned char out[MD5_DIGEST_LEN]);

// The same, written as lower-case hex: the form the signature takes in the URL.
void md5_hex(const void *data, size_t len, char out[MD5_HEX_LEN]);

// Incremental checksum for Archive file integrity, not authenticity. Source
// metadata arrives over verified TLS. Existing one-shot callers are unchanged.
typedef struct { uint32_t state[4]; uint64_t bytes; unsigned char block[64]; size_t have; } md5_ctx;
void md5_init(md5_ctx *ctx);
void md5_update(md5_ctx *ctx, const void *data, size_t len);
void md5_final_hex(md5_ctx *ctx, char out[MD5_HEX_LEN]);
#endif /* MD5_H */
