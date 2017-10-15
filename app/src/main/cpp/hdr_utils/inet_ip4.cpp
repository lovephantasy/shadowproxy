//
// Created by lovefantasy on 17-6-12.
//

#include <endian.h>
#include "inet_ip4.h"

u_int8_t inet_ip4::getVersion() {

    return (u_int8_t) (((*data) >> 4) & 0x0f);
}

u_int32_t inet_ip4::getSourceAddress() {
    return *((u_int32_t *) data + 3);
}
u_int32_t inet_ip4::getDestinationAddress(char *buf) {
    return *((u_int32_t *) buf + 4);
}
u_int32_t inet_ip4::getDestinationAddress() {
    return *((u_int32_t *) data + 4);
}
void inet_ip4::setSourceAddress(char *buf, u_int32_t src) {
    *((u_int32_t *) buf + 3) = src;
}
void inet_ip4::setSourceAddress(u_int32_t srcaddr) {
    *((u_int32_t *) data + 3) = srcaddr;
}
void inet_ip4::setDestinationAddress(char *buf, u_int32_t dst) {
    *((u_int32_t *) buf + 4) = dst;

}

void inet_ip4::setDestinationAddress(u_int32_t dstaddr) {
    *((u_int32_t *) data + 4) = dstaddr;
}

void inet_ip4::setChecksum(u_int16_t cksum) {
    *((u_int16_t *) data + 5) = cksum;
}

void inet_ip4::setChecksum(char *buf, u_int16_t cksum) {
    *((u_int16_t *) buf + 5) = cksum;

}

inet_ip4::inet_ip4(char *buffer) {
    this->data = buffer;

}

uint8_t inet_ip4::getProtocol() {
    return (u_int8_t) *(data + 9);
}

/***
 *  返回上层数据真实长度
 */
 u_int16_t inet_ip4::getDataLength() {
    /**
     * 总长度需考虑字节序，并且不需要考虑4
     */
    return ntohs(*((u_int16_t *) data + 1)) - (getHeaderLength() << 2);
}

u_int16_t inet_ip4::getDataLength(char *buf) {
    return ntohs(*((u_int16_t *) buf + 1)) - (getHeaderLength(buf) << 2);
}


/**
 *
 * @return 返回真实1/4头部长度
 */
u_int8_t inet_ip4::getHeaderLength() {
    return (u_int8_t) ((*data) & 0xf);
}

u_int8_t inet_ip4::getHeaderLength(char *buf) {
    return (u_int8_t) ((*buf) & 0xf);
}

/**
 *
 * @return 返回ip packet真实长度
 */
u_int16_t inet_ip4::getTotalLength() {
    return ntohs(*((u_int16_t *) data + 1));
}






