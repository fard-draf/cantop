#ifndef PROVIDERS_H_
#define PROVIDERS_H_
#include "analyzer.h"
#include "log.h"
#include "utils.h"

int provider_finder(Analyzer *an, CanReader *cr);
void provider_init(Analyzer *an, Provider *provider, CanReader *cr);
void provider_update(Analyzer *an, Provider *provider, CanReader *cr);
void providers_actives(Analyzer *an, Provider *provider);

#endif // PROVIDERS_H_
