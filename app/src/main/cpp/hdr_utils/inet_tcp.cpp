//
// Created by lovefantasy on 17-6-12.
//

#include <endian.h>
#include "inet_tcp.h"

u_int16_t inet_tcp::getSourcePort() {
    return *((u_int16_t *) data + 10);
}

u_int16_t inet_tcp::getDestinationPort() {
    return *((u_int16_t *) data + 11);
}

void inet_tcp::setSourcePort(u_int16_t srcport) {
    *((u_int16_t *) data + 10) = srcport;
}

void inet_tcp::setDestinationPort(u_int16_t dstport) {
    *((u_int16_t *) data + 11) = dstport;
}

void inet_tcp::setChecksum(u_int16_t cksum) {
    *((u_int16_t *) data + 18) = cksum;
}

inet_tcp::inet_tcp(char *buffer) {
    this->data = buffer;

}

/**
 *
 * @return 返回真实1/4头部长度
 */
u_int8_t inet_tcp::getHeaderLength() {
    return (u_int8_t) *(data + 32) >> 4;
}
