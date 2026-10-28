#ifndef SONIX_COLLECTION_HEADER_H
#define SONIX_COLLECTION_HEADER_H
#include "lvgl/lvgl.h"
/* Shared reference-layout content. Parent owns vertical flow and top clearance. */
typedef struct {
    lv_obj_t *cover, *title, *artist, *meta, *actions, *buttons[3];
} collection_header_t;
void collection_header_build(lv_obj_t *parent, int width, const char *title,
    const char *artist, const char *meta, const lv_image_dsc_t *primary,
    const lv_image_dsc_t *secondary, const lv_image_dsc_t *tertiary,
    lv_event_cb_t callbacks[3], void *user, collection_header_t *out);
#endif
