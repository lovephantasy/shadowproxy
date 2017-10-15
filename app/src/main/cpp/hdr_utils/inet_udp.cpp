//
// Created by lovefantasy on 17-6-12.
//

#include <endian.h>
#include "inet_udp.h"
u_int16_t inet_udp::getSourcePort(char *buf) {
    return *((u_int16_t *) buf + 10);
}

u_int16_t inet_udp::getSourcePort() {
    return *((u_int16_t *) data + 10);
}

u_int16_t inet_udp::getDestinationPort(char *buf) {
    return  *((u_int16_t *) buf + 11);

}

u_int16_t inet_udp::getDestinationPort() {
    return *((u_int16_t *) data + 11);
}
void inet_udp::setSourcePort(char *buf, u_int16_t src) {
    *((u_int16_t *) buf + 10) = src;

}
void inet_udp::setSourcePort(u_int16_t srcprot) {
    *((u_int16_t *) data + 10) = srcprot;
}
void inet_udp::setDestinationPort(char *buf, u_int16_t dst) {
    *((u_int16_t *) buf + 11) = dst;

}

void inet_udp::setDestinationPort(u_int16_t dstport) {
    *((u_int16_t *) data + 11) = dstport;
}



void inet_udp::setChecksum(u_int16_t cksum) {
    *((u_int16_t *) data + 13) = cksum;
}

void inet_udp::setChecksum(char *buf, u_int16_t cksum) {
    *((u_int16_t *) buf + 13) = cksum;
}

inet_udp::inet_udp(char *buffer) {

    this->data = buffer;

}

/**
 *
 * @return 返回包的真实长度
 */
u_int16_t inet_udp::getTotalLength() {
    /**
     * 16位以上的需要 考虑字节序？？
     */
    return htons(*((u_int16_t *) data + 12));
}

u_int16_t inet_udp::getTotalLength(char *buf) {
    return htons(*((u_int16_t *) buf + 12));
}

void inet_udp::setTotalLength(char *buf, u_int16_t len) {
    *((u_int16_t *) buf + 12) = htons(len);
}






