#ifndef SIGNALS_H_
#define SIGNALS_H_

#include "app.h"
#include <stdlib.h>
#include <sys/signal.h>

int sig_init(AppContext *ac);
void sig_handler(void *ctx);

#endif // SIGNALS_H_
