#include "../include/frame_parser.h"
#include <linux/can/raw.h>

#define SA_SHIFT 0
#define PGN_SHIFT 8
#define DEST_ADDR_SHIFT 8
#define PF_SHIFT 16
#define PRIORITY_SHIFT 26

#define PRIORITY_MASK 0x7
#define DEST_ADDR_MASK 0xFF
#define PF_MASK 0xFF
#define SA_MASK 0xFF
#define PGN_PDU1_MASK 0x3FF00
#define PGN_PDU2_MASK 0x3FFFF

#define J1939_PDU1_MAX_PF 0xEFU
#define J1939_BROADCAST_ADDR 0xFFU

static void pgn_builder(CanReader *cr, canid_t id);
static void sa_builder(CanReader *cr, canid_t id);
static void data_builder(CanReader *cr, struct can_frame *frame);
static void dlc_builder(CanReader *cr, struct can_frame *frame);
static void priority_builder(CanReader *cr, struct can_frame *frame);

void frame_reader(AppContext *ac) {
    canid_t id = ac->frame.can_id & CAN_EFF_MASK;

    pgn_builder(&ac->cr, id);
    dlc_builder(&ac->cr, &ac->frame);
    sa_builder(&ac->cr, id);
    priority_builder(&ac->cr, &ac->frame);
    data_builder(&ac->cr, &ac->frame);
}

static void pgn_builder(CanReader *cr, canid_t id) {

    cr->pf = (id >> PF_SHIFT) & PF_MASK;
    if (cr->pf > J1939_PDU1_MAX_PF) {
        /*broadcast*/
        cr->dest_addr = J1939_BROADCAST_ADDR;
        cr->pgn = (id >> PGN_SHIFT) & PGN_PDU2_MASK;
    } else {
        /*p2p message*/
        cr->dest_addr = (id >> DEST_ADDR_SHIFT) & DEST_ADDR_MASK;
        cr->pgn = (id >> PGN_SHIFT) & PGN_PDU1_MASK;
    }
}

static void sa_builder(CanReader *cr, canid_t id) {
    cr->sa = (id >> SA_SHIFT) & SA_MASK;
}

static void data_builder(CanReader *cr, struct can_frame *frame) {
    for (uint8_t i = 0; i < cr->dlc; i++) {
        cr->data[i] = frame->data[i];
    }
}

static void dlc_builder(CanReader *cr, struct can_frame *frame) {
    cr->dlc = frame->len;
}

static void priority_builder(CanReader *cr, struct can_frame *frame) {
    cr->priority = (frame->can_id >> PRIORITY_SHIFT) & PRIORITY_MASK;
}
