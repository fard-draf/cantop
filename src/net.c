#include <linux/can.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/un.h>

#include "../include/app.h"
#include "../include/frame_parser.h"
#include "../include/log.h"
#include "../include/net.h"

static const char *ifname = "can0";

ResultCanNet net_init(AppContext *ac) {
    int sfd;

    struct sockaddr_can addr;
    struct ifreq ifr;

    sfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sfd == -1) {
        int err = errno;
        syslog(LOG_ERR, "socket: %s", strerror(err));
        return INIT_ERR;
    }

    ac->net_fd = sfd;

    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
    if (ioctl(sfd, SIOCGIFINDEX, &ifr) == -1) {
        int err = errno;
        syslog(LOG_ERR, "ioctl: %s", strerror(err));
        return RUN_ERR;
    }

    syslog(LOG_INFO, "ifindex of %s = %d", ifname, ifr.ifr_ifru.ifru_ivalue);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        int err = errno;
        syslog(LOG_ERR, "bind: %s", strerror(err));
        return RUN_ERR;
    }

    return SUCCESS;
}

void net_recv(AppContext *ac) {
    ssize_t num_read = read(ac->net_fd, &ac->frame, sizeof(struct can_frame));
    if (num_read == -1) {
        int err = errno;
        if (err == EINTR) {
            return;
        } else {
            syslog(LOG_ERR, "[ERR] num_read err: %s", strerror(err));
            // TODO: reload on next tram -> stop for the moment
            running = 0;
            return;
        }
    }
}
