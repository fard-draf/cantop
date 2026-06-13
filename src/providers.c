#include "../include/providers.h"

int provider_finder(Analyzer *an, CanReader *cr) {
    // match source address
    for (uint8_t i = an->providers.reg.instances_actives_count; i-- > 0;) {
        if (an->providers.entries[i].sa == cr->sa) {
            syslog(LOG_DEBUG, "[PROVIDER %d] matching [PROV_INSTANCE %d] ",
                   cr->sa, i);
            an->providers.reg.is_fresh = false;
            return i;
        }
    }
    // if no match, take the first free
    if (an->providers.reg.instances_actives_count < MAX_PROVIDERS_INSTANCES) {
        an->providers.reg.is_fresh = true;
        return an->providers.reg.instances_actives_count;
    }

    // no place founded
    syslog(LOG_DEBUG, "[PROVIDER %d] -> NO SPACE LEFT IN [PROV_INSTANCE]",
           cr->sa);
    return -1;
}

void provider_init(Analyzer *an, ProviderEntry *provider, CanReader *cr) {
    provider->sa = cr->sa;
    provider->metr.datas.last_seen = elapsed_ms(&an->metr.start);
}

void provider_update(Analyzer *an, ProviderEntry *provider, CanReader *cr) {
    provider->metr.datas.last_seen = elapsed_ms(&an->metr.start);
}

void provider_update_rate(Analyzer *an, size_t provider_idx) {
    ProviderEntry *provider_entry = &an->providers.entries[provider_idx];
    int64_t now = elapsed_ms(&an->metr.start);
    int64_t dt = now - provider_entry->metr.datas.snapshot_time;
    uint32_t dc = provider_entry->metr.datas.tram_count_total -
                  provider_entry->metr.datas.tram_count_snapshot;

    if (dt > 0) {
        provider_entry->metr.datas.tram_rate = (dc * 1000) / dt;
    }

    provider_entry->metr.datas.tram_count_snapshot =
        provider_entry->metr.datas.tram_count_total;
    provider_entry->metr.datas.snapshot_time = now;
}
