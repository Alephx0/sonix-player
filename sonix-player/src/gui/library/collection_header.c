#include "collection_header.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/theme.h"
#include "src/gui/shell/ui_metrics.h"
#include <string.h>
static lv_obj_t *line(lv_obj_t *p, int width, const char *text, const lv_font_t *font, int lines) {
    lv_obj_t *l=lv_label_create(p);
    lv_label_set_text(l,text);
    lv_obj_set_width(l,width);
    lv_obj_set_style_text_font(l,font,0);
    lv_obj_set_style_text_color(l,lv_color_white(),0);
    lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_LEFT,0);
    lv_point_t size;lv_text_get_size(&size,text,font,0,0,width,LV_TEXT_FLAG_NONE);
    int height=lv_font_get_line_height(font);
    int needed=(size.y+height-1)/height;
    if(needed<1)needed=1;
    if(needed>lines)needed=lines;
    lv_obj_set_height(l,needed*height);
    lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);
    lv_obj_set_clickable(l,false);
    return l;
}
void collection_header_build(lv_obj_t *parent,int width,const char *title,const char *artist,
    const char *meta,const lv_image_dsc_t *primary,const lv_image_dsc_t *secondary,
    const lv_image_dsc_t *tertiary,lv_event_cb_t callbacks[3],void *user,collection_header_t *out) {
    memset(out,0,sizeof(*out));
    out->cover=lv_image_create(parent);
    lv_obj_set_size(out->cover,UI_COLLECTION_COVER,UI_COLLECTION_COVER);
    lv_obj_add_style(out->cover,&theme_style_card,0);
    lv_obj_set_style_pad_all(out->cover,0,0);
    lv_image_set_inner_align(out->cover,LV_IMAGE_ALIGN_CONTAIN);
    lv_image_set_src(out->cover,&icon_album_big);
    lv_obj_add_style(out->cover,&theme_style_icon,0);
    lv_obj_set_style_image_recolor_opa(out->cover,LV_OPA_COVER,0);
    lv_obj_set_clickable(out->cover,false);
    lv_obj_set_style_margin_bottom(out->cover,10,0);
    out->title=line(parent,width,title,&font_ui_28,2);
    if(artist && *artist)out->artist=line(parent,width,artist,&font_ui_18,1);
    if(meta && *meta){out->meta=line(parent,width,meta,&font_ui_20,1);lv_obj_add_style(out->meta,&theme_style_text_dim,0);}
    lv_obj_t *actions=out->actions=lv_obj_create(parent);
    lv_obj_remove_style_all(actions);
    lv_obj_set_size(actions,width,UI_COLLECTION_ACTION_HEIGHT);
    lv_obj_set_flex_flow(actions,LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actions,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(actions,20,0);
    lv_obj_set_style_margin_top(actions,12,0);
    lv_obj_set_scrollable(actions,false);lv_obj_set_clickable(actions,false);lv_obj_set_event_bubble(actions,true);
    const lv_image_dsc_t *icons[]={secondary,primary,tertiary};
    for(int i=0;i<3;i++) {
        if(!icons[i])continue;
        lv_obj_t *b=out->buttons[i]=lv_btn_create(actions);
        lv_obj_remove_style_all(b);
        lv_obj_set_size(b,i==1?UI_COLLECTION_PLAY:56,i==1?UI_COLLECTION_PLAY:56);
        lv_obj_set_clickable(b,true);lv_obj_set_event_bubble(b,true);
        lv_obj_set_style_radius(b,LV_RADIUS_CIRCLE,0);
        lv_obj_set_style_bg_opa(b,i==1?LV_OPA_COVER:LV_OPA_TRANSP,0);
        lv_obj_set_style_bg_color(b,lv_color_hex(0x1ed760),0);
        lv_obj_set_style_opa(b,LV_OPA_70,LV_STATE_PRESSED);
        if(callbacks[i])lv_obj_add_event_cb(b,callbacks[i],LV_EVENT_CLICKED,user);
        lv_obj_t *im=lv_image_create(b);lv_image_set_src(im,icons[i]);
        lv_obj_set_size(im,UI_COLLECTION_PLAY_GLYPH,UI_COLLECTION_PLAY_GLYPH);
        lv_image_set_inner_align(im,LV_IMAGE_ALIGN_CONTAIN);
        lv_obj_set_style_image_recolor(im,lv_color_hex(i==1?0x121212:0xffffff),0);
        lv_obj_set_style_image_recolor_opa(im,LV_OPA_COVER,0);lv_obj_set_clickable(im,false);lv_obj_center(im);
        if(i==1){lv_obj_set_ignore_layout(b,true);lv_obj_align(b,LV_ALIGN_RIGHT_MID,-4,0);}
    }
}
