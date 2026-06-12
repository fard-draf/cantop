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
