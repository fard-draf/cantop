#ifndef PGNS_H_
#define PGNS_H_
#include "analyzer.h"
#include "log.h"
#include "utils.h"

int pgn_instance_finder(Analyzer *an, CanReader *cr, size_t provider_idx);
void pgn_instance_init(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr);
void pgn_instance_update(Analyzer *an, PgnEntry *pgn_instance, CanReader *cr);

#endif // PGNS_H_
