#include "../include/reaper.h"

void reaper_pgns(Analyzer *an) {
    uint8_t last_entry = an->pgns.reg.instances_actives_count;
    for (uint8_t i = last_entry; i-- > 0;) {
        uint64_t dt = elapsed_ms(&an->metr.start) -
                      an->pgns.entries[i].metr.datas.last_seen;
        if (an->pgns.entries[i].metr.datas.tram_rate == 0 && dt > 5000) {
            an->pgns.entries[i] = an->pgns.entries[last_entry - 1];
            an->pgns.reg.instances_actives_count -= 1;
        }
    }
}

void reaper_providers(Analyzer *an) {
    uint8_t last_entry = an->providers.reg.instances_actives_count;
    for (uint8_t i = last_entry; i-- > 0;) {
        uint64_t dt = elapsed_ms(&an->metr.start) -
                      an->providers.entries[i].metr.datas.last_seen;
        if (an->providers.entries[i].metr.datas.tram_rate == 0 && dt > 5000) {
            an->providers.entries[i] = an->providers.entries[last_entry - 1];
            an->providers.reg.instances_actives_count -= 1;
        }
    }
}
