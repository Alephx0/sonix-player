#include "bottomsheet.h"
#include "ui_metrics.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/switcher.h"
static lv_obj_t *overlay, *panel, *body;
static void (*dispose)(void);
static int resting_y, press_y;
static bool closing;
static void position(void *o, int32_t y) { lv_obj_set_y(o, y); }
static void opacity(void *o, int32_t v) { lv_obj_set_style_bg_opa(o, v, 0); }
static void destroy(void) {
	if (dispose) {
		void (*fn)(void) = dispose;
		dispose = NULL;
		fn();
	}
	if (overlay)
		lv_obj_delete(overlay);
	overlay = panel = body = NULL;
	closing = false;
	back_gesture_blocked(false);
}
static void finished(lv_anim_t *a) {
	(void)a;
	destroy();
}
static void animate(lv_obj_t *o, lv_anim_exec_xcb_t fn, int from, int to, bool exit) {
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, o);
	lv_anim_set_exec_cb(&a, fn);
	lv_anim_set_values(&a, from, to);
	lv_anim_set_duration(&a, UI_SHEET_TRANSITION_MS);
	lv_anim_set_path_cb(&a, exit ? lv_anim_path_ease_in : lv_anim_path_ease_out);
	if (exit && o == panel)
		lv_anim_set_completed_cb(&a, finished);
	lv_anim_start(&a);
}
void bottomsheet_close(bool animated) {
	if (!overlay)
		return;
	if (closing) {
		if (!animated)
			destroy();
		return;
	}
	lv_anim_delete(panel, NULL);
	lv_anim_delete(overlay, NULL);
	if (!animated) {
		destroy();
		return;
	}
	closing = true;
	animate(overlay, opacity, lv_obj_get_style_bg_opa(overlay, 0), 0, true);
	animate(panel, position, lv_obj_get_y(panel), lv_display_get_vertical_resolution(NULL), true);
}
static void dismiss(lv_event_t *e) {
	(void)e;
	bottomsheet_close(true);
}
static void drag(lv_event_t *e) {
	lv_indev_t *in = lv_indev_active();
	if (!in || closing)
		return;
	lv_point_t p;
	lv_indev_get_point(in, &p);
	if (lv_event_get_code(e) == LV_EVENT_PRESSED) {
		press_y = p.y;
		lv_anim_delete(panel, NULL);
	} else if (lv_event_get_code(e) == LV_EVENT_PRESSING)
		lv_obj_set_y(panel, resting_y + LV_MAX(0, p.y - press_y));
	else if (p.y - press_y > 70)
		bottomsheet_close(true);
	else
		animate(panel, position, lv_obj_get_y(panel), resting_y, false);
}
bool bottomsheet_active(void) { return overlay != NULL; }
lv_obj_t *bottomsheet_open(int height, void (*cleanup)(void)) {
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
	bool fresh = !overlay;
	if (closing) {
		destroy();
		fresh = true;
	}
	if (!fresh) {
		lv_anim_delete(panel, NULL);
		lv_anim_delete(overlay, NULL);
		if (dispose) {
			void (*fn)(void) = dispose;
			dispose = NULL;
			fn();
		}
		lv_obj_clean(body);
	} else {
		overlay = lv_obj_create(lv_layer_top());
		lv_obj_remove_style_all(overlay);
		lv_obj_set_size(overlay, w, h);
		lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
		lv_obj_set_scrollable(overlay, false);
		lv_obj_set_clickable(overlay, true);
		lv_obj_add_event_cb(overlay, dismiss, LV_EVENT_CLICKED, NULL);
		panel = lv_obj_create(overlay);
		lv_obj_remove_style_all(panel);
		lv_obj_set_style_bg_color(panel, lv_color_hex(0x202124), 0);
		lv_obj_set_style_bg_opa(panel, 255, 0);
		lv_obj_set_style_radius(panel, 22, 0);
		lv_obj_set_scrollable(panel, false);
		lv_obj_set_clickable(panel, true);
		lv_obj_t *handle = lv_obj_create(panel);
		lv_obj_remove_style_all(handle);
		lv_obj_set_scrollable(handle, false);
		lv_obj_set_size(handle, w - 64, 32);
		lv_obj_set_pos(handle, 0, 0);
		lv_obj_set_clickable(handle, true);
		lv_obj_add_event_cb(handle, drag, LV_EVENT_PRESSED, NULL);
		lv_obj_add_event_cb(handle, drag, LV_EVENT_PRESSING, NULL);
		lv_obj_add_event_cb(handle, drag, LV_EVENT_RELEASED, NULL);
		lv_obj_add_event_cb(handle, drag, LV_EVENT_PRESS_LOST, NULL);
		lv_obj_t *bar = lv_obj_create(handle);
		lv_obj_remove_style_all(bar);
		lv_obj_set_size(bar, 48, 5);
		lv_obj_set_clickable(bar, false);
		lv_obj_set_scrollable(bar, false);
		lv_obj_set_style_bg_color(bar, lv_color_hex(0x727477), 0);
		lv_obj_set_style_bg_opa(bar, 255, 0);
		lv_obj_set_style_radius(bar, 3, 0);
		lv_obj_set_pos(bar, w / 2 - 24, 12);
		lv_obj_t *close = lv_obj_create(panel);
		lv_obj_remove_style_all(close);
		lv_obj_set_size(close, 56, 40);
		lv_obj_set_pos(close, w - 60, 0);
		lv_obj_set_clickable(close, true);
		lv_obj_add_event_cb(close, dismiss, LV_EVENT_CLICKED, NULL);
		lv_obj_t *im = lv_image_create(close);
		lv_image_set_src(im, &icon_close);
		lv_obj_set_size(im, 20, 20);
		lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_center(im);
		body = lv_obj_create(panel);
		lv_obj_remove_style_all(body);
		lv_obj_set_pos(body, 0, 36);
		lv_obj_set_scrollable(body, false);
		lv_obj_set_clickable(body, true);
	}
	height = LV_MIN(h - 40, LV_MAX(160, height));
	resting_y = h - height;
	lv_obj_set_size(panel, w, height);
	lv_obj_set_size(body, w, height - 36);
	dispose = cleanup;
	back_gesture_blocked(true);
	lv_obj_set_style_bg_opa(overlay, 150, 0);
	lv_obj_set_y(panel, resting_y);
	if (fresh) {
		animate(panel, position, h, resting_y, false);
		animate(overlay, opacity, 0, 150, false);
	}
	lv_obj_update_layout(panel);
	return body;
}
