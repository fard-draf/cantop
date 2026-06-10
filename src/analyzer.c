#include "../include/analyzer.h"
#include "../include/log.h"
#include "../include/pgns.h"
#include "../include/providers.h"
// TUI refresh
static int first_run = 1;

int analyzer_init(Analyzer *self) {
    memset(self, 0, sizeof(Analyzer));
    clock_gettime(CLOCK_MONOTONIC, &self->g_metr.start);

    /* set the true flag for all provider instance after memset */
    for (size_t i = 0; i < MAX_PROVIDERS_NBR; i++) {
        self->prov_inst[i].is_free = true;
        for (size_t j = 0; j < MAX_PROVIDER_INSTANCES; j++) {
            self->prov_inst[i].pgn_inst[j].is_free = true;
        }
    }

    syslog(LOG_INFO, "analyzer started..");
    return 0;
}

int analyzer_populate(Analyzer *self, CanReader *cr) {
    int provider_idx, pgn_idx;

    self->g_metr.last_seen = elapsed_ms(&self->g_metr.start);
    self->g_metr.tram_count_total += 1;
    /* searching for a place inside providers,
     * either a match, either a fresh place */
    provider_idx = prov_instance_finder(self, cr);
    if (provider_idx == -1) {
        self->g_metr.tram_count_err += 1;
        return -1;
    }

    Provider *prov_instance = &self->prov_inst[provider_idx];

    /* pgn integration on an instance
     *
     * if unmatch, provider init and writing on the first slot */
    if (prov_instance->is_free == true) {
        prov_instance_init(self, prov_instance, cr);
        pgn_idx = 0;
    } else {
        /* if match:
         *     searching for match provider->pgn on an active instance
         *     or creating a new one */
        pgn_idx = pgn_instance_finder(self, cr, provider_idx);
        if (pgn_idx == -1) {
            self->g_metr.tram_count_err += 1;
            return -1;
        }
        prov_instance_update(self, prov_instance, cr);
    }

    /* here, pgn_instance is either an existing instance or a fresh one */
    PgnEntry *pgn_instance = &prov_instance->pgn_inst[pgn_idx];

    /* if unmatch, init a pgn instance */
    if (pgn_instance->is_free == true) {
        pgn_instance_init(self, pgn_instance, cr);
    } else {
        /* if match founded, update it */
        pgn_instance_update(self, pgn_instance, cr);
    }

    return 0;
}

void analyzer_update_rate(Analyzer *self) {
    int64_t now = elapsed_ms(&self->g_metr.start);
    /*delta time*/
    int64_t dt = now - self->g_metr.snapshot_time;
    /*delta count*/
    uint32_t dc =
        self->g_metr.tram_count_total - self->g_metr.tram_count_snapshot;

    if (dt > 0) {
        self->g_metr.tram_rate = dc / (dt / 1000);
    }

    self->g_metr.tram_count_snapshot = self->g_metr.tram_count_total;
    self->g_metr.snapshot_time = now;
}
