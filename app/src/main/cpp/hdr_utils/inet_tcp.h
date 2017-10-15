//
// Created by lovefantasy on 17-6-12.
//

#ifndef SHADOWPROXY_INET_TCP_H
#define SHADOWPROXY_INET_TCP_H


#include <sys/types.h>
#pragma pack(push,1)
class inet_tcp {
private:
    /***
    u_int16_t sourceport;
    u_int16_t destinationport;

    u_int32_t sequencenum;

    u_int32_t acknowledgenum;

    u_int8_t headerlength;   //前4
    u_int8_t flag;    //后6
    u_int16_t windwsize;

    u_int16_t checksum;
    u_int16_t urgentpointer;

    u_int32_t options;

    ssize_t dataoffset;
    ssize_t datalength;
*/

public:
    char *data;

    inet_tcp(char *buffer);

    u_int16_t getSourcePort();

    u_int16_t getDestinationPort();

    u_int8_t getHeaderLength();

    void setSourcePort(u_int16_t);

    void setDestinationPort(u_int16_t);

    void setChecksum(u_int16_t);

};
#pragma pack(pop)


#endif //SHADOWPROXY_INET_TCP_H
