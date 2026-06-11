#include "../include/analyzer.h"
#include "../include/log.h"
#include "../include/pgns.h"
#include "../include/providers.h"
// TUI refresh
static int first_run = 1;

int analyzer_init(Analyzer *an) {
    memset(an, 0, sizeof(Analyzer));
    clock_gettime(CLOCK_MONOTONIC, &an->g_metr.start);

    /* set the true flag for all provider instance after memset */
    for (size_t i = 0; i < MAX_PROVIDERS_NBR; i++) {
        an->prov_inst[i].is_free = true;
        for (size_t j = 0; j < MAX_PROVIDER_INSTANCES; j++) {
            an->prov_inst[i].pgn_inst[j].is_free = true;
        }
    }

    syslog(LOG_INFO, "analyzer started..");
    return 0;
}

int analyzer_populate(Analyzer *an, CanReader *cr) {
    int provider_idx, pgn_idx;

    an->g_metr.last_seen = elapsed_ms(&an->g_metr.start);
    an->g_metr.tram_count_total += 1;
    /* searching for a place inside providers,
     * either a match, either a fresh place */
    provider_idx = provider_finder(an, cr);
    if (provider_idx == -1) {
        an->g_metr.tram_count_err += 1;
        return -1;
    }

    Provider *provider = &an->prov_inst[provider_idx];

    /* pgn integration on an instance
     *
     * if unmatch, provider init and writing on the first slot */
    if (provider->is_free == true) {
        provider_init(an, provider, cr);
        pgn_idx = 0;
    } else {
        /* if match:
         *     searching for match provider->pgn on an active instance
         *     or creating a new one */
        pgn_idx = pgn_finder(an, cr, provider_idx);
        if (pgn_idx == -1) {
            an->g_metr.tram_count_err += 1;
            return -1;
        }
        provider_update(an, provider, cr);
    }

    /* here, pgn_entry is either an existing instance or a fresh one */
    PgnEntry *pgn_entry = &provider->pgn_inst[pgn_idx];

    /* if unmatch, init a pgn instance */
    if (pgn_entry->is_free == true) {
        pgn_init(an, pgn_entry, cr);
    } else {
        /* if match founded, update it */
        pgn_update(an, pgn_entry, cr);
    }

    return 0;
}

void analyzer_update_rate(Analyzer *an) {
    int64_t now = elapsed_ms(&an->g_metr.start);
    int64_t dt = now - an->g_metr.snapshot_time;
    uint32_t dc = an->g_metr.tram_count_total - an->g_metr.tram_count_snapshot;

    if (dt > 0) {
        an->g_metr.tram_rate = dc / (dt / 1000);
    }

    an->g_metr.tram_count_snapshot = an->g_metr.tram_count_total;
    an->g_metr.snapshot_time = now;
}
