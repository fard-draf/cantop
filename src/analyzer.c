#include "../include/analyzer.h"
#include "../include/log.h"
#include "../include/pgns.h"
#include "../include/providers.h"
// TUI refresh
static int first_run = 1;

int analyzer_init(Analyzer *an) {
    memset(an, 0, sizeof(Analyzer));
    clock_gettime(CLOCK_MONOTONIC, &an->metr.start);
    syslog(LOG_INFO, "analyzer started..");
    return 0;
}

int analyzer_populate(Analyzer *an, CanReader *cr) {
    int provider_idx, pgn_idx;

    an->metr.glob_data_rate.last_seen = elapsed_ms(&an->metr.start);
    an->metr.glob_data_rate.tram_count_total += 1;
    /* searching for a place inside providers,
     * either a match, either a fresh place
     * => return the index of prov_inst*/
    provider_idx = provider_finder(an, cr);
    if (provider_idx == -1) {
        an->metr.glob_data_rate.tram_count_err += 1;
        return -1;
    }

    Provider *provider = &an->prov_inst[provider_idx];

    /* pgn integration on an instance*/
    if (an->prov_mgmt.is_fresh == true) {
        /* if unmatch, provider init and writing on the next free slot */
        provider_init(an, provider, cr);
        an->prov_mgmt.instances_actives += 1;
        pgn_idx = 0;
    } else {
        /* if match:
         *     update an existing provider instance
         *     or creating a new one */
        provider_update(an, provider, cr);
        /* searching for a place inside pgns instances,
         * either a match, either a fresh place
         * => return the index of pgn_inst */
        pgn_idx = pgn_finder(an, cr, provider_idx);
        if (pgn_idx == -1) {
            an->metr.glob_data_rate.tram_count_err += 1;
            return -1;
        }
    }

    /* here, pgn_entry is either an existing instance or a fresh one */
    PgnEntry *pgn_entry = &an->pgn_inst[pgn_idx];

    /* if is a fresh one, init a pgn instance */
    if (an->pgn_mgmt.is_fresh == true) {
        pgn_init(an, pgn_entry, cr);
        an->pgn_mgmt.instances_actives += 1;
    } else {
        /* if match founded, update it */
        pgn_update(an, pgn_entry, cr);
        pgn_entry->metr.pgn_data_rate.tram_count_total += 1;
    }

    return 0;
}

void analyzer_update_rate(Analyzer *an) {
    int64_t now = elapsed_ms(&an->metr.start);
    int64_t dt = now - an->metr.glob_data_rate.snapshot_time;
    uint32_t dc = an->metr.glob_data_rate.tram_count_total -
                  an->metr.glob_data_rate.tram_count_snapshot;

    if (dt > 0) {
        an->metr.glob_data_rate.tram_rate = dc / (dt / 1000);
    }

    an->metr.glob_data_rate.tram_count_snapshot =
        an->metr.glob_data_rate.tram_count_total;
    an->metr.glob_data_rate.snapshot_time = now;
}
