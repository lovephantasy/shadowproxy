//
// Created by lovefantasy on 17-6-12.
//

#include "nat_mgr.h"


/***
 *  tcp_session manager
 */
tcpsession *natmgr::get_tcp_session(u_int16_t key) {
    map<u_int16_t, tcpsession>::iterator it = tcp_sessions->find(key);
    if (it != tcp_sessions->end()) {
        return &(it->second);
    } else {
        return NULL;

    }
}

tcpsession *natmgr::create_tcp_session(u_int32_t remoteip, u_int16_t remoteport, u_int16_t sourceport) {

    tcpsession *session = (tcpsession *) malloc(sizeof(tcpsession));
    session->remote_ip = remoteip;
    session->remote_port = remoteport;
    session->source_port=sourceport;
    tcp_sessions->insert(map<u_int16_t, tcpsession>::value_type(sourceport, *session));
    return session;
}

void natmgr::remove_tcp_session(u_int16_t key) {
    tcp_sessions->erase(key);
}

/***
 *  udp_session manager
 */
/*
tcpsession *natmgr::get_udp_session(u_int16_t key) {

    map<u_int16_t, tcpsession>::iterator it = udp_sessions->find(key);
    if (it != udp_sessions->end()) {
        return &(it->second);
    } else {
        return NULL;

    }
}

tcpsession *natmgr::create_udp_session(u_int32_t remote_ip, u_int16_t remote_port,
                                       u_int32_t sourceip, u_int16_t sourceport,
                                       char *host) {

    tcpsession *session = new tcpsession();
    session->remote_ip = remote_ip;
    session->remote_port = remote_port;

    session->sourceip = sourceip;
    session->sourceport = sourceport;

    udp_sessions->insert(map<u_int16_t, tcpsession>::value_type(sourceport, *session));

    return session;
}

void natmgr::remove_udp_session(u_int16_t key) {
    udp_sessions->erase(key);
}

void natmgr::init() {
    tcp_sessions = new map<u_int16_t, tcpsession>;
    udp_sessions = new map<u_int16_t, tcpsession>;
}
*/