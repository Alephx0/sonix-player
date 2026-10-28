#include "online_album.h"
#include "src/gui/library/collection_header.h"
#include "src/gui/shell/main_menu.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/ui_metrics.h"
#include "src/gui/fonts/fonts.h"
#include "src/system/discovery/download_queue.h"
#include <stdio.h>
#include <stdint.h>
static lv_obj_t *screen,*list;
static discovery_results_t release;
static gui_config_t *config;
static online_track_cb_t open_track;
bool online_album_active(void) {return screen && lv_screen_active()==screen;}
static lv_obj_t *label(lv_obj_t *parent,const char *text,int w,int h,const lv_font_t *font,uint32_t color) {
    lv_obj_t *l=lv_label_create(parent);lv_label_set_text(l,text);lv_obj_set_size(l,w,h);
    lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_style_text_font(l,font,0);
    lv_obj_set_style_text_color(l,lv_color_hex(color),0);lv_obj_set_clickable(l,false);return l;
}
static lv_obj_t *button(lv_obj_t *parent,const char *text,int w,int h,lv_event_cb_t cb,void *data) {
    lv_obj_t *b=lv_obj_create(parent);lv_obj_remove_style_all(b);lv_obj_set_size(b,w,h);
    lv_obj_set_scrollable(b,false);lv_obj_set_clickable(b,true);lv_obj_set_event_bubble(b,true);
    if(cb)lv_obj_add_event_cb(b,cb,LV_EVENT_CLICKED,data);
    if(text)lv_obj_center(label(b,text,w-16,28,&font_ui_18,0xffffff));
    return b;
}
static void track_cb(lv_event_t *e) {if(open_track)open_track(lv_event_get_user_data(e));}
static void downloads_cb(lv_event_t *e) {(void)e;main_menu_open_downloads();}
static void download_album_cb(lv_event_t *e) {
    (void)e;
    if(!gui_card_available()){gui_notify_no_card();return;}
    char error[192]="Could not open download queue.";
    if(!download_queue_init(config->sd_root_path) || !download_enqueue_release(&release,error,sizeof(error))) {
        gui_notify_popup(error);return;
    }
    main_menu_open_downloads();
}
static void loaded(lv_event_t *e) {(void)e;main_menu_mount_footer(screen);}
static void unloaded(lv_event_t *e) {(void)e;main_menu_restore_footer(screen);}
void online_album_open(gui_config_t *cfg,const discovery_results_t *results,unsigned page,
    online_track_cb_t track,lv_event_cb_t page_cb,lv_event_cb_t artist_cb) {
    if(!results || !results->count)return;
    release=*results;config=cfg;open_track=track;
    int w=cfg->screen_width-2*cfg->padding;
    if(!screen) {
        screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);
        lv_obj_set_size(screen,cfg->screen_width,cfg->screen_height);
        lv_obj_set_style_bg_color(screen,lv_color_hex(0x121212),0);lv_obj_set_style_bg_opa(screen,255,0);
        list=lv_obj_create(screen);lv_obj_remove_style_all(list);
        lv_obj_set_pos(list,cfg->padding,cfg->top_bar_height);
        lv_obj_set_size(list,w,cfg->screen_height-cfg->top_bar_height-146);
        lv_obj_set_flex_flow(list,LV_FLEX_FLOW_COLUMN);lv_obj_set_scroll_dir(list,LV_DIR_VER);
        lv_obj_add_event_cb(screen,loaded,LV_EVENT_SCREEN_LOADED,NULL);
        lv_obj_add_event_cb(screen,unloaded,LV_EVENT_SCREEN_UNLOADED,NULL);
    }
    lv_obj_clean(list);lv_obj_scroll_to_y(list,0,LV_ANIM_OFF);
    lv_obj_t *head=lv_obj_create(list);lv_obj_remove_style_all(head);
    lv_obj_set_size(head,w,LV_SIZE_CONTENT);lv_obj_set_flex_flow(head,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(head,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(head,6,0);lv_obj_set_style_pad_top(head,cfg->padding+(56-icon_chevron_left.header.h)/2,0);
    lv_obj_set_style_pad_bottom(head,12,0);lv_obj_set_scrollable(head,false);lv_obj_set_clickable(head,false);
    char meta[128];snprintf(meta,sizeof(meta),"Internet Archive / %d %s%s",release.count,release.count==1?"file":"files",release.truncated?" on this page":"");
    collection_header_t h;lv_event_cb_t callbacks[]={artist_cb,download_album_cb,downloads_cb};
    collection_header_build(head,w,release.items[0].album,release.items[0].artist,meta,
        &icon_download,&icon_artist,&icon_list_queue,callbacks,NULL,&h);
    /* Never imply that this page is an entire release when pagination remains. */
    if(page!=1 || release.truncated) {
        lv_obj_add_state(h.buttons[1],LV_STATE_DISABLED);lv_obj_set_style_opa(h.buttons[1],LV_OPA_30,LV_STATE_DISABLED);
        label(list,"Download individual files on paged releases",w,28,&font_ui_16,0xb3b3b3);
    }
    for(int i=0;i<release.count;i++) {
        discovery_item_t *item=&release.items[i];
        lv_obj_t *row=button(list,NULL,w,UI_COLLECTION_TRACK_HEIGHT,track_cb,item);
        char number[12];snprintf(number,sizeof(number),"%u",(page-1)*DISCOVERY_MAX_RESULTS+i+1);
        lv_obj_set_pos(label(row,number,32,28,&font_ui_18,0xb3b3b3),0,12);
        lv_obj_set_pos(label(row,item->title,w-56,30,&font_ui_20,0xffffff),42,8);
        char sub[180];snprintf(sub,sizeof(sub),"%s / %s / %.1f MB",item->artist,item->format,(double)item->bytes/1048576);
        lv_obj_set_pos(label(row,sub,w-56,24,&font_ui_16,0xb3b3b3),42,40);
    }
    if(page>1)button(list,"Previous files",w,52,page_cb,(void *)(intptr_t)-1);
    if(release.truncated)button(list,"More files",w,52,page_cb,(void *)(intptr_t)1);
    if(lv_screen_active()!=screen)switch_screen(screen);
}
