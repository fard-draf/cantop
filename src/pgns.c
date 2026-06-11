#include "../include/pgns.h"

int pgns_finder(Analyzer *an, CanReader *cr, size_t provider_idx) {
    // match pgn between inst[i].frame.pgn and cr->pgn
    for (size_t i = 0; i < MAX_PROVIDER_INSTANCES; i++) {
        if (an->prov_inst[provider_idx].pgn_inst[i].frame.pgn == cr->pgn) {
            syslog(LOG_DEBUG,
                   "[PROVIDER %d] [INSTANCE %zu] match [PGN_INSTANCE %zu] for"
                   "[PGN %d]",
                   cr->sa, provider_idx, i, cr->pgn);
            return i;
        }
    }

    // if not, we iterate to find the first free instance for pgn
    for (size_t i = 0; i < MAX_PROVIDER_INSTANCES; i++) {
        if (an->prov_inst[provider_idx].pgn_inst[i].is_free) {
            syslog(LOG_DEBUG,
                   "[PROVIDER %d] [INSTANCE %zu] create new [PGN_INSTANCE %zu] "
                   "for [PGN %d]",
                   cr->sa, provider_idx, i, cr->pgn);
            return i;
        }
    }

    // no left space
    syslog(LOG_DEBUG, "[PROVIDER %d] [INSTANCE %zu] -> NO SPACE LEFT", cr->pgn,
           provider_idx);
    return -1;
}

void pgns_init(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr) {
    pgn_instance->is_free = false;
    pgn_instance->frame.pgn = cr->pgn;
    pgn_instance->frame.data_len = cr->can_dlc;
    pgn_instance->frame.count += 1;
    pgn_instance->metr.time_start = elapsed_ms(&an->g_metr.start);
    pgn_instance->metr.last_seen = pgn_instance->metr.time_start;
}

void pgns_update(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr) {
    pgn_instance->metr.last_seen = elapsed_ms(&an->g_metr.start);
    pgn_instance->frame.count += 1;
    pgn_instance->frame.data_len = cr->can_dlc;
}
