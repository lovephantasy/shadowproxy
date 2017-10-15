//
// Created by lovefantasy on 17-6-12.
//

#ifndef SHADOWPROXY_NATSESSIONMANAGER_H
#define SHADOWPROXY_NATSESSIONMANAGER_H

#include <map>
#include "nat_session.h"

using namespace std;
#pragma pack(push,1)
class natmgr {
public:
    static map<u_int16_t, tcpsession> *tcp_sessions;
    static map<u_int16_t, tcpsession> *udp_sessions;

public :
    //static void init();

    static tcpsession *get_tcp_session(u_int16_t);

    static tcpsession *create_tcp_session(u_int32_t, u_int16_t, u_int16_t);

    static void remove_tcp_session(u_int16_t);

    //static tcpsession *get_udp_session(u_int16_t);

  //  static tcpsession *create_udp_session(u_int32_t, u_int16_t, u_int32_t, u_int16_t, char *);

   // static void remove_udp_session(u_int16_t);
};
#pragma pack(pop)


#endif //SHADOWPROXY_NATSEEIONMANAGER_H
