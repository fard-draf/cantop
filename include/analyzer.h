#ifndef ANALYZER_H_
#define ANALYZER_H_
#include "common.h"
#include "utils.h"

#define MAX_PROVIDERS_NBR 64
#define MAX_PROVIDER_INSTANCES 64

typedef struct {
    uint32_t pgn;
    uint32_t count;
    uint8_t data_len;
} ProviderPgn;

typedef struct {
    int64_t time_start;
    int64_t last_seen;
    int64_t snapshot_time;
    uint32_t tram_rate;
    uint32_t tram_count_snapshot;
    uint16_t count_err;
    uint16_t count_total;
} PgnMetrics;

typedef struct {
    ProviderPgn frame;
    PgnMetrics metr;
    bool is_free;
} PgnEntry;

typedef struct {
    int64_t time_start;
    int64_t last_seen;
    int64_t snapshot_time;
    uint32_t tram_rate;
    uint32_t tram_count_snapshot;
    uint8_t pgn_count_actual;
} ProviderMetrics;

typedef struct {
    PgnEntry pgn_inst[MAX_PROVIDER_INSTANCES];
    ProviderMetrics metr;
    uint8_t sa;
    bool is_free;
} Provider;

typedef struct {
    struct timespec start;
    int64_t last_seen;
    int64_t snapshot_time;
    uint32_t tram_rate;
    uint32_t tram_count_err;
    uint32_t tram_count_total;
    uint32_t tram_count_snapshot;
    uint8_t providers_count_max;
    uint8_t providers_count_actual;
} GlobMetrics;

typedef struct {
    Provider prov_inst[MAX_PROVIDERS_NBR];
    GlobMetrics g_metr;
} Analyzer;

int analyzer_populate(Analyzer *self, CanReader *cr);
int analyzer_init(Analyzer *self);
void analyzer_update_rate(Analyzer *self);

#endif // ANALYZER_H_
