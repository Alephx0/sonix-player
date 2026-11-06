#include <assert.h>
#include <stdio.h>
#include "src/system/input/touch_calibration.h"

typedef struct {
    struct input_absinfo axes[ABS_CNT];
    bool available[ABS_CNT];
} controller_t;

static bool read_axis(void *context, unsigned axis, struct input_absinfo *out) {
    controller_t *controller = context;
    if (!controller->available[axis]) return false;
    *out = controller->axes[axis];
    return true;
}

static void axis(controller_t *controller, unsigned code, int min, int max) {
    controller->available[code] = true;
    controller->axes[code].minimum = min;
    controller->axes[code].maximum = max;
}

int main(void) {
    controller_t controller = {0};
    touch_calibration_t c = {1, 2, 3, 4};
    assert(!touch_calibration_read(read_axis, &controller, &c));
    assert(c.min_x == 1 && c.max_y == 4); /* failure must not destroy calibration */

    /* A real MT-only controller has no legacy ABS_X/Y range. */
    axis(&controller, ABS_MT_POSITION_X, 0, 480);
    axis(&controller, ABS_MT_POSITION_Y, 0, 800);
    assert(touch_calibration_read(read_axis, &controller, &c));
    assert(c.min_x == 0 && c.min_y == 0 && c.max_x == 480 && c.max_y == 800);

    /* Unsupported legacy axes can succeed with degenerate ranges. */
    axis(&controller, ABS_X, 0, 0);
    axis(&controller, ABS_Y, 0, 0);
    assert(touch_calibration_read(read_axis, &controller, &c));
    assert(c.max_x == 480 && c.max_y == 800);

    /* Do not mix a usable legacy X with a missing/invalid legacy Y. */
    axis(&controller, ABS_X, 10, 1010);
    assert(touch_calibration_read(read_axis, &controller, &c));
    assert(c.min_x == 0 && c.max_x == 480);

    /* Single-touch controllers keep their measured, possibly nonzero range. */
    axis(&controller, ABS_Y, 20, 2020);
    assert(touch_calibration_read(read_axis, &controller, &c));
    assert(c.min_x == 10 && c.min_y == 20 && c.max_x == 1010 && c.max_y == 2020);
    touch_calibration_t flipped = touch_calibration_oriented(c, true);
    assert(flipped.min_x == 1010 && flipped.min_y == 2020 && flipped.max_x == 10 && flipped.max_y == 20);
    touch_calibration_t restored = touch_calibration_oriented(c, false);
    assert(restored.min_x == 10 && restored.max_y == 2020);
    puts("touch calibration: MT-only, degenerate/mixed axes, legacy ranges and rotation passed");
    return 0;
}
