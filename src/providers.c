#include "../include/providers.h"

int provider_finder(Analyzer *an, CanReader *cr) {
    // match source address
    for (uint8_t i = an->prov_mgmt.instances_actives; i-- > 0;) {
        if (an->prov_inst[i].sa == cr->sa) {
            syslog(LOG_DEBUG, "[PROVIDER %d] matching [PROV_INSTANCE %d] ",
                   cr->sa, i);
            an->prov_mgmt.is_fresh = false;
            return i;
        }
    }
    // if no match, take the first free
    if (an->prov_mgmt.instances_actives < MAX_PROVIDERS_INSTANCES) {
        an->prov_mgmt.is_fresh = true;
        return an->prov_mgmt.instances_actives;
    }

    // no place founded
    syslog(LOG_DEBUG, "[PROVIDER %d] -> NO SPACE LEFT IN [PROV_INSTANCE]",
           cr->sa);
    return -1;
}

void provider_init(Analyzer *an, Provider *provider, CanReader *cr) {

    provider->sa = cr->sa;
    provider->metr.prov_data_rate.last_seen = elapsed_ms(&an->metr.start);
}

void provider_update(Analyzer *an, Provider *provider, CanReader *cr) {
    provider->metr.prov_data_rate.last_seen = elapsed_ms(&an->metr.start);
}

void providers_actives(Analyzer *an, Provider *provider) {
    /* searching for the flag !free on providers instances */
}
