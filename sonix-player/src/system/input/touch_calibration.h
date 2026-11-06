#ifndef SONIX_TOUCH_CALIBRATION_H
#define SONIX_TOUCH_CALIBRATION_H

#include <stdbool.h>
#include <linux/input.h>

typedef struct {
    int min_x, min_y, max_x, max_y;
} touch_calibration_t;

typedef bool (*touch_axis_reader_t)(void *context, unsigned axis, struct input_absinfo *info);

/* LVGL reads MT positions too, but its default range probe only asks for
 * ABS_X/Y. MT-only controllers therefore need an explicit calibration,
 * especially when physical pixels differ from the logical display size.
 * Keep each coordinate pair together; never combine different protocols. */
static inline bool touch_calibration_read(touch_axis_reader_t read_axis, void *context,
                                         touch_calibration_t *out) {
    const unsigned axes[][2] = {{ABS_X, ABS_Y}, {ABS_MT_POSITION_X, ABS_MT_POSITION_Y}};
    for (unsigned i = 0; i < sizeof(axes) / sizeof(axes[0]); ++i) {
        struct input_absinfo x = {0}, y = {0};
        if (read_axis(context, axes[i][0], &x) && read_axis(context, axes[i][1], &y) &&
            x.maximum > x.minimum && y.maximum > y.minimum) {
            *out = (touch_calibration_t){x.minimum, y.minimum, x.maximum, y.maximum};
            return true;
        }
    }
    return false;
}

static inline touch_calibration_t touch_calibration_oriented(touch_calibration_t raw, bool rotated) {
    if (rotated) {
        return (touch_calibration_t){raw.max_x, raw.max_y, raw.min_x, raw.min_y};
    }
    return raw;
}

#endif
