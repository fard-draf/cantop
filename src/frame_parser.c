#include "../include/frame_parser.h"
#include "../include/log.h"
#include <linux/can/raw.h>

#define PDU_THRESHOLD 240
#define PF_MASK 0xFF
#define PF_SHIFT 16
#define PGN_SHIFT 8
#define PGN_PDU1_MASK 0x3FF00
#define PGN_PDU2_MASK 0x3FFFF
#define SA_MASK 0xFF
#define SA_SHIFT 0

static void pgn_builder(CanReader *cr, canid_t id);
static void sa_builder(CanReader *cr, canid_t id);
static void data_builder(CanReader *cr, struct can_frame *frame);

void frame_reader(AppContext *ac) {
    canid_t id = ac->frame.can_id & CAN_EFF_MASK;
    pgn_builder(&ac->cr, id);
    ac->cr.can_dlc = ac->frame.can_dlc;
    sa_builder(&ac->cr, id);
    data_builder(&ac->cr, &ac->frame);
}

static void pgn_builder(CanReader *cr, canid_t id) {

    uint8_t pf = (id >> PF_SHIFT) & PF_MASK;
    if (pf >= PDU_THRESHOLD) {
        cr->pgn = (id >> PGN_SHIFT) & PGN_PDU2_MASK;
    } else {
        cr->pgn = (id >> PGN_SHIFT) & PGN_PDU1_MASK;
    }
}

static void sa_builder(CanReader *cr, canid_t id) {

    cr->sa = (id >> SA_SHIFT) & SA_MASK;
}

static void data_builder(CanReader *cr, struct can_frame *frame) {
    for (uint8_t i = 0; i < cr->can_dlc; i++) {
        cr->data[i] = frame->data[i];
    }
}
