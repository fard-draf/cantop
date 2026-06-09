#ifndef CAN_UTILS_H
#define CAN_UTILS_H

#include <linux/can/raw.h>
#include <stdint.h>

typedef struct {
    uint32_t pgn;
    uint8_t pf;
    uint8_t sa;
    uint8_t can_dlc;
    uint8_t data[8];
} CanReader;

#endif //  CAN_UTILS_H
