#ifndef SONIX_WEBP_SCALED_H
#define SONIX_WEBP_SCALED_H
#include <stddef.h>
#include <stdint.h>
/* Static, bounded decoder; RGB888 is malloc-owned by caller. */
uint8_t *webp_scaled_decode(const uint8_t *data,size_t size,int max_box,int *w,int *h);
#endif
