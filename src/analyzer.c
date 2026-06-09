#include "../include/analyzer.h"
#include "../include/log.h"

// TUI refresh
static int first_run = 1;

int64_t elapsed_ms(const struct timespec *start) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec - start->tv_sec) * 1000 +
           (now.tv_nsec - start->tv_nsec) / 1000000;
}

//==================================================================================GLOB
int analyzer_start_glob(Analyzer *self) {
    memset(self, 0, sizeof(Analyzer));
    clock_gettime(CLOCK_MONOTONIC, &self->g_metr.start);

    for (size_t i = 0; i < MAX_PROVIDERS_NBR; i++) {
        self->prov_inst[i].is_free = true;
        for (size_t j = 0; j < MAX_PROVIDER_INSTANCES; j++) {
            self->prov_inst[i].pgn_inst[j].is_free = true;
        }
    }

    syslog(LOG_INFO, "analyzer started..");
    return 0;
}

//==================================================================================PROV

int prov_instance_finder(Analyzer *an, CanReader *cr);
void prov_instance_init(Analyzer *an, Provider *prov_instance, CanReader *cr);
void prov_instance_update(Analyzer *an, Provider *prov_instance, CanReader *cr);

int pgn_instance_finder(Analyzer *an, CanReader *cr, size_t provider_idx);
void pgn_instance_init(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr);
void pgn_instance_update(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr);

int analyzer_populate(Analyzer *self, CanReader *cr) {
    int provider_idx, pgn_idx;

    self->g_metr.last_seen = elapsed_ms(&self->g_metr.start);
    self->g_metr.tram_count_total += 1;
    //== cherche une place dans les providers
    // soit un match, soit une place libre
    provider_idx = prov_instance_finder(self, cr);
    if (provider_idx == -1) {
        self->g_metr.tram_count_err += 1;
        return -1;
    }

    Provider *prov_instance = &self->prov_inst[provider_idx];

    //== integration du pgn dans une instance
    //== == si pas de match -> on initialise le provider puis on ecrit sur le
    // premier slot
    if (prov_instance->is_free == true) {
        self->g_metr.providers_count_actual += 1;
        prov_instance_init(self, prov_instance, cr);
        pgn_idx = 0;
    } else {
        //== == si un match, on cherche un match du provider->pgn sur une
        // instance ou une place
        // libre si ce pgn n est pas encore dans le tableau
        pgn_idx = pgn_instance_finder(self, cr, provider_idx);
        if (pgn_idx == -1) {
            self->g_metr.tram_count_err += 1;
            return -1;
        }
        prov_instance_update(self, prov_instance, cr);
    }

    // here, instance_idx is a new one or a match.
    PgnEntry *pgn_instance = &prov_instance->pgn_inst[pgn_idx];

    //== if no match -> initialize instance and write on the first one
    if (pgn_instance->is_free == true) {
        pgn_instance_init(self, pgn_instance, cr);
    } else {
        // else if is a match, we update
        pgn_instance_update(self, pgn_instance, cr);
    }

    self->g_metr.tram_count_valid += 1;

    return 0;
}

void analyzer_print(Analyzer *self) {

#define LINES 2
#define CLR "\r\033[2K"

    if (!first_run) {
        fprintf(stdout, "\033[%dA", LINES);
    }

    fprintf(stdout, CLR "> cantop | v0.1 |\n");
    fprintf(stdout, CLR "active_tram: %d, started_since: %ld\n",
            self->g_metr.tram_count_total, self->g_metr.last_seen);
    fflush(stdout);
    first_run = 0;
}
//==========================================HELPERS
//======================PROVIDER
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
    prov_instance->metr.pgn_count_actual = 1;

    prov_instance->is_free = false;
    prov_instance->sa = cr->sa;
}

void prov_instance_update(Analyzer *an, Provider *prov_instance,
                          CanReader *cr) {
    prov_instance->metr.last_seen = elapsed_ms(&an->g_metr.start);
    prov_instance->metr.pgn_count_actual += 1;
}
//======================INSTANCE
int pgn_instance_finder(Analyzer *an, CanReader *cr, size_t provider_idx) {
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

void pgn_instance_init(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr) {
    pgn_instance->is_free = false;
    pgn_instance->frame.pgn = cr->pgn;
    pgn_instance->frame.data_len = cr->can_dlc;
    pgn_instance->frame.count += 1;
    pgn_instance->metr.time_start = elapsed_ms(&an->g_metr.start);
    pgn_instance->metr.last_seen = pgn_instance->metr.time_start;
}

void pgn_instance_update(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr) {
    pgn_instance->metr.last_seen = elapsed_ms(&an->g_metr.start);
    pgn_instance->frame.count += 1;
    pgn_instance->frame.data_len = cr->can_dlc;
}
