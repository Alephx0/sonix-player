#include "preferences.h"
#include "src/system/core/config.h"
discovery_preferences_t discovery_preferences_defaults(void) {
    discovery_preferences_t value={true,0,{true,3,0,0}};return value;
}
discovery_preferences_t discovery_preferences_load(void) {
    discovery_preferences_t v=discovery_preferences_defaults();
    v.enabled=config_get_bool("discovery","enabled",true);
    v.format=(unsigned)config_get_int("discovery","format",0);
    if(v.format>2)v.format=0;
    v.transfers.start_immediately=config_get_bool("downloads","start_immediately",true);
    v.transfers.attempts=(unsigned)config_get_int("downloads","attempts",3);
    v.transfers.rate_kib=(unsigned)config_get_int("downloads","rate_kib",0);
    v.transfers.naming=(unsigned)config_get_int("downloads","naming",0);
    download_configure(&v.transfers);v.transfers=download_options();return v;
}
bool discovery_preferences_save(const discovery_preferences_t *v) {
    if(!v || v->format>2 || v->transfers.naming>2 || v->transfers.attempts<1 || v->transfers.attempts>3 ||
       (v->transfers.rate_kib && (v->transfers.rate_kib<64 || v->transfers.rate_kib>1024)))return false;
    discovery_preferences_t old=discovery_preferences_load();
    config_set_bool("discovery","enabled",v->enabled);
    config_set_int("discovery","format",v->format);
    config_set_bool("downloads","start_immediately",v->transfers.start_immediately);
    config_set_int("downloads","attempts",v->transfers.attempts);
    config_set_int("downloads","rate_kib",v->transfers.rate_kib);
    config_set_int("downloads","naming",v->transfers.naming);
    if(!config_save()) {
        config_set_bool("discovery","enabled",old.enabled);config_set_int("discovery","format",old.format);
        config_set_bool("downloads","start_immediately",old.transfers.start_immediately);
        config_set_int("downloads","attempts",old.transfers.attempts);
        config_set_int("downloads","rate_kib",old.transfers.rate_kib);config_set_int("downloads","naming",old.transfers.naming);
        return false;
    }
    download_configure(&v->transfers);return true;
}
