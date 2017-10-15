//
// Created by lovefantasy on 2017/6/4.
//

#ifndef SHADOWPROXY_HTTPDNS_H
#define SHADOWPROXY_HTTPDNS_H


#include <jni.h>

#include <linux/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <endian.h>
#include <arpa/inet.h>
#include <malloc.h>


#include "../utils/global.h"
#include "../utils/utils.h"
#include "../tunnel/tunnel.h"

#pragma pack(push,1)
typedef struct udp{
    int ip_hdr_len;
    epoll_event ev;
    char* buf:0;
    u_int16_t src_port:0;
    ssize_t recv_len;
    bool dns:false;
}udp_t;
#endif

#pragma pack(push,1)
class udp_ontcp :public tunnel{

private:
    udp_t* udp;
    char *m_data;
public:
    udp_ontcp(int epoll_fd, sockaddr_in *dst);

    int send_udp_packet(char *buf, int total_len);
    virtual void on_epoll_in() override;

    virtual void on_epoll_out() override;

    virtual void connect_remote() override;
     void close_connection();
    virtual ~udp_ontcp() override ;
};

#pragma pack(pop)


#endif //SHADOWPROXY_HTTPDNS_H
