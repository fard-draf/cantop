#include "../include/providers.h"

int prov_instance_finder(Analyzer *an, CanReader *cr) {
    // match source address
    for (size_t i = 0; i < MAX_PROVIDERS_NBR; i++) {
        if (an->prov_inst[i].sa == cr->sa) {
            syslog(LOG_DEBUG, "[PROVIDER %d] matching [PROV_INSTANCE %zu] ",
                   cr->sa, i);
            return i;
        }
    }
    // if no match, take the first free
    for (size_t i = 0; i < MAX_PROVIDERS_NBR; i++) {
        if (an->prov_inst[i].is_free == true) {
            syslog(LOG_DEBUG, "[PROVIDER %d] create new [PROV_INSTANCE %zu]",
                   cr->sa, i);
            return i;
        }
    }

    // no place founded
    syslog(LOG_INFO, "[PROVIDER %d] -> NO SPACE LEFT IN [PROV_INSTANCE]",
           cr->sa);
    return -1;
}

void prov_instance_init(Analyzer *an, Provider *prov_instance, CanReader *cr) {
    prov_instance->metr.time_start = elapsed_ms(&an->g_metr.start);
    prov_instance->metr.last_seen = prov_instance->metr.time_start;
    an->g_metr.providers_count_actual += 1;
    an->g_metr.providers_count_max += 1;
    prov_instance->is_free = false;
    prov_instance->sa = cr->sa;
}

void prov_instance_update(Analyzer *an, Provider *prov_instance,
                          CanReader *cr) {
    prov_instance->metr.last_seen = elapsed_ms(&an->g_metr.start);
    an->g_metr.providers_count_actual += 1;
}
