#include "../include/pgns.h"

int pgn_finder(Analyzer *an, CanReader *cr, size_t provider_idx) {
    Provider provider = an->prov_inst[provider_idx];

    // match pgn between inst[i].frame.pgn and cr->pgn
    for (uint8_t i = an->pgn_mgmt.instances_actives; i-- > 0;) {
        if (an->pgn_inst[i].frame.pgn == cr->pgn) {

            syslog(LOG_DEBUG,
                   "[DEBUG] provider %d | instance %zu | pgn %d -> match "
                   "instance on pgn_instance %d",
                   cr->sa, provider_idx, cr->pgn, i);

            an->pgn_mgmt.is_fresh = false;
            return i;
        }
    }

    // if unmatch, we iterate to find the first free instance for pgn
    if (an->pgn_mgmt.instances_actives < MAX_PGNS_INSTANCES) {

        syslog(LOG_DEBUG,
               "[DEBUG] an->%d | instance %zu | pgn %d -> create new"
               "instance %d",
               cr->sa, provider_idx, cr->pgn, an->pgn_mgmt.instances_actives);

        an->pgn_mgmt.is_fresh = true;
        return an->pgn_mgmt.instances_actives;
    }

    // not enought space
    syslog(LOG_DEBUG,
           "[DEBUG] provider %d | instance %zu | pgn %d -> not enought space",
           cr->sa, provider_idx, cr->pgn);
    return -1;
}

void pgn_init(Analyzer *an, PgnEntry *pgn_entry, CanReader *cr) {
    pgn_entry->frame.pgn = cr->pgn;
    pgn_entry->frame.data_len = cr->can_dlc;
    pgn_entry->frame.count += 1;
    pgn_entry->metr.pgn_data_rate.last_seen = elapsed_ms(&an->metr.start);
    for (uint8_t i = 0; i < pgn_entry->frame.data_len; i++) {
        pgn_entry->frame.data[i] = cr->data[i];
    }
}

void pgn_update(Analyzer *an, PgnEntry *pgn_entry, CanReader *cr) {
    pgn_entry->metr.pgn_data_rate.last_seen = elapsed_ms(&an->metr.start);
    pgn_entry->frame.count += 1;
    pgn_entry->frame.data_len = cr->can_dlc;
    for (uint8_t i = 0; i < pgn_entry->frame.data_len; i++) {
        pgn_entry->frame.data[i] = cr->data[i];
    }
}
