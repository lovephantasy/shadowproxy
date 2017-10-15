//
// Created by lovefantasy on 2017/6/3.
//

#ifndef SHADOWPROXY_UTILS_H
#define SHADOWPROXY_UTILS_H


#include <sys/epoll.h>
#include "../hdr_utils/inet_ip4.h"
#include "../hdr_utils/inet_tcp.h"
#include "../hdr_utils/inet_udp.h"

#include "global.h"
#include <endian.h>
#include <fcntl.h>
#include <stdio.h>

#pragma pack(push,1)
class utils {
public:
    static void updateipchecksum(inet_ip4 *hdr) {
        u_int16_t *buf = (u_int16_t *) hdr->data;
        hdr->setChecksum(0);
        int len = hdr->getHeaderLength() << 2;
        hdr->setChecksum(cksum(buf, 0, len));
    };

    static void updateipchecksum(char *data) {
        u_int16_t *buf = (u_int16_t *) data;
        inet_ip4::setChecksum(data, 0);
        int len = inet_ip4::getHeaderLength(data) << 2;
        inet_ip4::setChecksum(data, cksum(buf, 0, len));
    };

    /**
     *结果有时多0x100?????
     *
     * 解决：严格按照16位，来计算。
     */
    static void updatetcpchecksum(inet_ip4 *iphdr, inet_tcp *tcphdr) {
        int len = iphdr->getDataLength();
        u_int32_t sum = *((u_int16_t *) iphdr->data + 6) +
                        *((u_int16_t *) iphdr->data + 7) +
                        *((u_int16_t *) iphdr->data + 8) +
                        *((u_int16_t *) iphdr->data + 9) +
                        htons((u_int16_t) 6) +  ///这里？？？？？
                        htons(iphdr->getDataLength());
        /***
        * 16bit的都需要字节序转换网络序
        */
        /* __android_log_print(ANDROID_LOG_ERROR, "len", "header: %d data:%d",
                             tcphdr->getHeaderLength() * 4, iphdr->getDataLength());*/
        u_int16_t *addr = (u_int16_t *) (iphdr->data + (iphdr->getHeaderLength() << 2));
        tcphdr->setChecksum(0);

        for (; len > 1; len -= 2)
            sum += *addr++;
        if (len > 0)
            sum += *(u_int8_t *) addr;

        while (sum >> 16)
            sum = (sum >> 16) + (sum & 0xffff);
        tcphdr->setChecksum((u_int16_t) ~sum);

    };

    static void updateudpchecksum(inet_ip4 *iphdr, inet_udp *udphdr) {
        int len = udphdr->getTotalLength();
        //__android_log_print(ANDROID_LOG_ERROR, "len", "udp: %d ", len);
        u_int32_t sum = *((u_int16_t *) iphdr->data + 6) +
                        *((u_int16_t *) iphdr->data + 7) +
                        *((u_int16_t *) iphdr->data + 8) +
                        *((u_int16_t *) iphdr->data + 9) +
                        htons((u_int16_t) 17) +
                        htons(iphdr->getDataLength());

        u_int16_t *addr = (u_int16_t *) (iphdr->data + (iphdr->getHeaderLength() << 2));

        udphdr->setChecksum(0);

        for (; len > 1; len -= 2)
            sum += *addr++;
        if (len > 0)
            sum += *(u_int8_t *) addr;
        while (sum >> 16)
            sum = (sum >> 16) + (sum & 0xffff);

        udphdr->setChecksum((u_int16_t) ~sum);

    };

    static void updateudpchecksum(char *buf) {

        u_int32_t sum = 0;
        int len = inet_udp::getTotalLength(buf);
        //__android_log_print(ANDROID_LOG_ERROR, "len", "udp: %d ", len);
        sum += *((u_int16_t *) buf + 6) +
               *((u_int16_t *) buf + 7) +
               *((u_int16_t *) buf + 8) +
               *((u_int16_t *) buf + 9) +
               htons((u_int16_t) 17) +
               htons(inet_ip4::getDataLength(buf));

        u_int16_t *addr = (u_int16_t *) (buf + (inet_ip4::getHeaderLength(buf) << 2));

        inet_udp::setChecksum(buf, 0);

        for (; len > 1; len -= 2)
            sum += *addr++;
        if (len > 0)
            sum += (*(u_int8_t *) addr);
        while (sum >> 16)
            sum = (sum >> 16) + (sum & 0xffff);

        inet_udp::setChecksum(buf, (u_int16_t) ~sum);

    };

    static u_int16_t cksum(u_int16_t *data, int offset, int len) {
        u_int32_t sum = 0;
        u_int16_t *buf = data + offset;
        for (sum = 0; len > 1; len -= 2)
            sum += *buf++;
        if (len > 0)
            sum += *(u_int8_t *) buf;
        while (sum >> 16)
            sum = (sum >> 16) + (sum & 0xffff);
        return (u_int16_t) ~sum;

    };
    /*static u_int8_t get_type(const char *data) {
        switch (*data) {
            case 'G':
                return HTTP;
            case 'P':
                if (*(data + 1) == 'O')
                    return HTTP;
                else
                    return HTTP_OTHERS;///PATCH
            case 0x16:
                return SSL;
            case 'C':
                if (*(data + 2) == 'N')
                    return HTTP_CONNECT;
                else
                    return HTTP_OTHERS;///COPY
            case 'O':   ///OPTIONS
                return HTTP_OTHERS;
            case 'M':   ///MOVE
                return HTTP_OTHERS;
            case 'H':   ///HEAD
                return HTTP_OTHERS;
            case 'T':   ///TRACE
                return HTTP_OTHERS;
            case 'D':   ///DELETE
                return HTTP_OTHERS;
            case 'L':   ///LINK
                return HTTP_OTHERS;
            case 'U':   ///UNLINK
                return HTTP_OTHERS;
            case 'W':   ///WRAPPED
                return HTTP_OTHERS;
            default:
                return SSL;
        }
    };*/

};
#pragma pack(pop)


#endif //SHADOWPROXY_UTILS_H
