#include "../include/pgns.h"

static void pgn_history_push(PgnEntry *e) {
    DataSnapshot *s = &e->history[e->history_head];
    s->data_len = e->frame.data_len;
    memcpy(s->data, e->frame.data, e->frame.data_len);
    e->history_head = (e->history_head + 1) % PGN_DATA_HISTORY;
    if (e->history_count < PGN_DATA_HISTORY)
        e->history_count++;
}

int pgn_finder(Analyzer *an, CanReader *cr, size_t provider_idx) {
    ProviderEntry provider = an->providers.entries[provider_idx];

    // match pgn between inst[i].frame.pgn and cr->pgn
    for (uint8_t i = an->pgns.reg.instances_actives_count; i-- > 0;) {
        if (an->pgns.entries[i].frame.pgn == cr->pgn &&
            an->pgns.entries[i].provider == cr->sa) {

            for (uint8_t j = an->providers.reg.instances_actives_count;
                 j-- > 0;) {
                if (an->providers.entries[j].sa == cr->sa) {
                    an->providers.entries[j].metr.datas.tram_count_total++;
                }
            }
            syslog(LOG_DEBUG,
                   "[DEBUG] provider %d | instance %zu | pgn %d -> match "
                   "instance on pgn_instance %d",
                   cr->sa, provider_idx, cr->pgn, i);

            an->pgns.reg.is_fresh = false;
            return i;
        }
    }

    // if unmatch, we iterate to find the first free instance for pgn
    if (an->pgns.reg.instances_actives_count < MAX_PGNS_INSTANCES) {

        syslog(LOG_DEBUG,
               "[DEBUG] an->%d | instance %zu | pgn %d -> create new"
               "instance %d",
               cr->sa, provider_idx, cr->pgn,
               an->pgns.reg.instances_actives_count);

        an->pgns.reg.is_fresh = true;
        return an->pgns.reg.instances_actives_count;
    }

    // not enought space
    syslog(LOG_DEBUG,
           "[DEBUG] provider %d | instance %zu | pgn %d -> not enought space",
           cr->sa, provider_idx, cr->pgn);
    return -1;
}

void pgn_init(Analyzer *an, PgnEntry *pgn_entry, CanReader *cr) {
    pgn_entry->history_head  = 0;
    pgn_entry->history_count = 0;
    pgn_entry->frame.pgn = cr->pgn;
    pgn_entry->frame.data_len = cr->dlc;
    pgn_entry->frame.dest_addr = cr->dest_addr;
    pgn_entry->frame.priority = cr->priority;
    pgn_entry->frame.count += 1;
    pgn_entry->provider = cr->sa;
    pgn_entry->metr.datas.last_seen = elapsed_ms(&an->metr.start);
    for (uint8_t i = 0; i < pgn_entry->frame.data_len; i++) {
        pgn_entry->frame.data[i] = cr->data[i];
    }
    pgn_history_push(pgn_entry);
}

void pgn_update(Analyzer *an, PgnEntry *pgn_entry, CanReader *cr) {
    pgn_entry->metr.datas.last_seen = elapsed_ms(&an->metr.start);
    pgn_entry->frame.count += 1;
    pgn_entry->frame.data_len = cr->dlc;
    for (uint8_t i = 0; i < pgn_entry->frame.data_len; i++) {
        pgn_entry->frame.data[i] = cr->data[i];
    }
    pgn_history_push(pgn_entry);
}

void pgn_update_rate(Analyzer *an, size_t pgn_idx) {
    PgnEntry *pgn_entry = &an->pgns.entries[pgn_idx];
    int64_t now = elapsed_ms(&an->metr.start);
    int64_t dt = now - pgn_entry->metr.datas.snapshot_time;
    uint32_t dc = pgn_entry->metr.datas.tram_count_total -
                  pgn_entry->metr.datas.tram_count_snapshot;

    if (dt > 0) {
        pgn_entry->metr.datas.tram_rate = (dc * 1000) / dt;
    }

    pgn_entry->metr.datas.tram_count_snapshot =
        pgn_entry->metr.datas.tram_count_total;
    pgn_entry->metr.datas.snapshot_time = now;
}
