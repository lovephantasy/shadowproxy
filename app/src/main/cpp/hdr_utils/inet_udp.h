//
// Created by lovefantasy on 17-6-12.
//

#ifndef SHADOWPROXY_INET_UDP_H
#define SHADOWPROXY_INET_UDP_H


#include <sys/types.h>
#pragma pack(push,1)
class inet_udp {
private:
    /***
    u_int16_t sourceport;
    u_int16_t destinationport;

    u_int16_t totallength;
    u_int16_t checksum;

    ssize_t dataoffset;
    ssize_t datalength;
*/
public:
    char *data;

    inet_udp(char *buffer);

    u_int16_t getTotalLength();

    static u_int16_t getTotalLength(char *);
    static void setTotalLength(char *,u_int16_t);

    static u_int16_t getSourcePort(char* buf);

    u_int16_t getSourcePort();

    static u_int16_t getDestinationPort(char* buf);
    u_int16_t getDestinationPort();
    static void setSourcePort(char*, u_int16_t);
    void setSourcePort(u_int16_t);

    static void setDestinationPort(char*,u_int16_t);

    void setDestinationPort(u_int16_t);

    void setChecksum(u_int16_t);

    static void setChecksum(char *, u_int16_t);

};
#pragma pack(pop)


#endif //SHADOWPROXY_INET_UDP_H
