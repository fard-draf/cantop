#ifndef PROVIDERS_H_
#define PROVIDERS_H_
#include "analyzer.h"
#include "log.h"
#include "utils.h"

int prov_instance_finder(Analyzer *an, CanReader *cr);
void prov_instance_init(Analyzer *an, Provider *prov_instance, CanReader *cr);
void prov_instance_update(Analyzer *an, Provider *prov_instance, CanReader *cr);

#endif // PROVIDERS_H_
