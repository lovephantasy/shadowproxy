//
// Created by lovefantasy on 17-6-12.
//

#ifndef SHADOWPROXY_INET_IP4_H
#define SHADOWPROXY_INET_IP4_H

#include <sys/types.h>
#pragma pack(push,1)
class inet_ip4 {
private:
   /***
    u_int8_t version;//前4位
    u_int8_t headerlength;//后4位
    u_int8_t typeofservice;
    u_int16_t totallength;

    u_int16_t identifier;
    u_int16_t flagsandfragmentedoffset;

    u_int8_t timetolive;
    u_int8_t protocol;
    u_int16_t checksum;

    u_int32_t sourceaddress;

    u_int32_t destinationaddress;

    u_int32_t options;

    ssize_t dataoffset;
    ssize_t datalength;

*/
public:
    char *data;

    inet_ip4(char *buffer);

    u_int8_t getVersion();

    u_int8_t getHeaderLength();

    static u_int8_t getHeaderLength(char *);

    u_int16_t getTotalLength();

     u_int16_t getDataLength();

    static u_int16_t getDataLength(char *);

     uint8_t getProtocol();

    u_int32_t getSourceAddress();

    static u_int32_t getDestinationAddress(char* buf);

    u_int32_t getDestinationAddress();

    static void setSourceAddress(char*,u_int32_t);
    void setSourceAddress(u_int32_t);
    static void setDestinationAddress(char*,u_int32_t);
    void setDestinationAddress(u_int32_t);

    void setChecksum(u_int16_t);

    static void setChecksum(char*,u_int16_t);

};
#pragma pack(pop)


#endif //SHADOWPROXY_INET_IP4_H
