//
// Created by lovefantasy on 17-6-12.
//

#ifndef SHADOWPROXY_NATSESSION_H
#define SHADOWPROXY_NATSESSION_H


#include <sys/types.h>
#include "global.h"
#pragma pack(push,1)
typedef struct struct_tcpsession {

    u_int8_t request_type=0;

    u_int32_t remote_ip;
    u_int16_t remote_port;

    u_int16_t source_port;
    uint64_t bytes_send;
    uint64_t packets_send;

} tcpsession;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct struct_udpsession {

    u_int16_t source_port;


} udpsession;
#pragma pack(pop)
#endif //SHADOWPROXY_NATSESSION_H
