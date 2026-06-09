#ifndef NET_H_
#define NET_H_
#include "app.h"
#include "common.h"

typedef enum {
    SUCCESS = 0,
    INTERR_ERR = 1,
    RUN_ERR = -1,
    INIT_ERR = -2,
} ResultCanNet;

ResultCanNet net_init(AppContext *ac);
void net_recv(AppContext *ac);

#endif // NET_CAN_H_
