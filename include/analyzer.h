#ifndef ANALYZER_H_
#define ANALYZER_H_
#include "common.h"
#include "utils.h"

#define MAX_PROVIDERS_INSTANCES 255
#define MAX_PGNS_INSTANCES 255

typedef struct {
    union {
        int8_t providers_actives_idx[MAX_PROVIDERS_INSTANCES];
        int8_t pgns_actives_idx[MAX_PGNS_INSTANCES];
    };
    uint8_t instances_actives;
    bool is_fresh;
} KeyManager;

typedef struct {
    int64_t snapshot_time;
    uint64_t last_seen;
    uint32_t tram_rate;
    uint32_t tram_count_total;
    uint32_t tram_count_err;
    uint32_t tram_count_snapshot;
} DataRate;

typedef struct {
    uint32_t pgn;
    uint32_t count;
    uint8_t data_len;
    uint8_t data[8];
} Frame;

typedef struct {
    DataRate pgn_data_rate;
} PgnMetrics;

typedef struct {
    Frame frame;
    PgnMetrics metr;
    uint8_t provider;
} PgnEntry;

typedef struct {
    DataRate prov_data_rate;
} ProviderMetrics;

typedef struct {
    ProviderMetrics metr;
    uint8_t sa;
} Provider;

typedef struct {
    DataRate glob_data_rate;
    struct timespec start;
} GlobMetrics;

typedef struct {
    Provider prov_inst[MAX_PROVIDERS_INSTANCES];
    PgnEntry pgn_inst[MAX_PGNS_INSTANCES];
    GlobMetrics metr;
    KeyManager pgn_mgmt;
    KeyManager prov_mgmt;
} Analyzer;

int analyzer_populate(Analyzer *an, CanReader *cr);
int analyzer_init(Analyzer *an);
void analyzer_update_rate(Analyzer *an);

#endif // ANALYZER_H_
