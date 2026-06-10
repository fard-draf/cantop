#ifndef NET_H_
#define NET_H_
#include "common.h"
#include "context.h"

int net_init(AppContext *ac);
void net_recv(AppContext *ac);

#endif // NET_CAN_H_
