#ifndef CAN_UTILS_H
#define CAN_UTILS_H

#include <linux/can/raw.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    uint32_t pgn;
    uint8_t pf;
    uint8_t sa;
    uint8_t dlc;
    uint8_t data[8];
    uint8_t priority;
    uint8_t dest_addr;
} CanReader;

int64_t elapsed_ms(const struct timespec *start);

#endif //  CAN_UTILS_H
