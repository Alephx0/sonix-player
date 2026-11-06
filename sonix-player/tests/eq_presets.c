#include "src/system/audio/eq.h"
#include "src/system/core/config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    assert(argc == 3);
    char config[1024];
    snprintf(config, sizeof(config), "%s/config.ini", argv[1]);
    config_init(config);
    eq_presets_set_dir(argv[1]);
    if (!strcmp(argv[2], "save")) {
        const char *required[] = {"eq_pop", "eq_rock", "R&B", "eq_acoustic"};
        for (unsigned r=0;r<sizeof(required)/sizeof(required[0]);r++) {
            int found=0;
            for (int i=0;i<EQ_DEFAULT_PRESET_COUNT;i++) {
                if (strcmp(required[r],EQ_DEFAULT_PRESETS[i].name)) continue;
                eq_apply_preset(&EQ_DEFAULT_PRESETS[i]);
                for (int b=0;b<EQ_BANDS;b++)
                    assert(eq_get_band_tenths(b)==10*EQ_DEFAULT_PRESETS[i].gain_db[b]);
                found=1;
            }
            assert(found);
        }
        for (int b=0;b<EQ_BANDS;b++) eq_set_band(b,b-5);
        assert(eq_preset_save("My listening"));
        assert(!eq_preset_save("../escape"));
    } else {
        assert(eq_preset_load("My listening"));
        for (int b=0;b<EQ_BANDS;b++) assert(eq_get_band_tenths(b)==10*(b-5));
        assert(eq_preset_delete("My listening"));
        assert(!eq_preset_load("My listening"));
    }
    puts("PASS EQ preset curves / custom persistence");
}
