#ifndef PROVIDERS_H_
#define PROVIDERS_H_
#include "analyzer.h"
#include "log.h"
#include "utils.h"

int provider_finder(Analyzer *an, CanReader *cr);
void provider_init(Analyzer *an, ProviderEntry *provider, CanReader *cr);
void provider_update(Analyzer *an, ProviderEntry *provider, CanReader *cr);
void provider_update_rate(Analyzer *an, size_t provider_idx);

#endif // PROVIDERS_H_
