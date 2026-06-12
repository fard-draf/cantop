#ifndef PGNS_H_
#define PGNS_H_
#include "analyzer.h"
#include "log.h"
#include "utils.h"

int pgn_finder(Analyzer *an, CanReader *cr, size_t provider_idx);
void pgn_init(Analyzer *an, PgnEntry *pgn_entry, CanReader *cr);
void pgn_update(Analyzer *an, PgnEntry *pgn_entry, CanReader *cr);
void pgn_update_rate(Analyzer *an, size_t pgn_idx);

#endif // PGNS_H_
