#ifndef ANALYZER_H_
#define ANALYZER_H_
#include "common.h"
#include "utils.h"

#define MAX_PROVIDERS_INSTANCES 255
#define MAX_PGNS_INSTANCES 255

typedef struct {
    uint8_t instances_actives_count;
    bool is_fresh;
} Registry;

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
    uint8_t priority;
    uint8_t dest_addr;
} Frame;

typedef struct {
    DataRate datas;
} PgnMetrics;

typedef struct {
    Frame frame;
    PgnMetrics metr;
    uint8_t provider;
} PgnEntry;

typedef struct {
    PgnEntry entries[MAX_PGNS_INSTANCES];
    Registry reg;
} Pgns;

typedef struct {
    DataRate datas;
} ProviderMetrics;

typedef struct {
    ProviderMetrics metr;
    uint8_t sa;
} ProviderEntry;

typedef struct {
    ProviderEntry entries[MAX_PROVIDERS_INSTANCES];
    Registry reg;
} Providers;

typedef struct {
    DataRate datas;
    struct timespec start;
} GlobMetrics;

typedef struct {
    Providers providers;
    Pgns pgns;
    GlobMetrics metr;
} Analyzer;

int analyzer_populate(Analyzer *an, CanReader *cr);
int analyzer_init(Analyzer *an);
void analyzer_update_rate(Analyzer *an);

#endif // ANALYZER_H_
