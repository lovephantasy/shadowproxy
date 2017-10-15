//
// Created by lovefantasy on 2017/6/4.
//

#ifndef SHADOWPROXY_UDPREALY_H
#define SHADOWPROXY_UDPREALY_H

#include <jni.h>
#include <linux/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <endian.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

#include "../tunnel/tunnel.h"
#include "../utils/global.h"
#pragma pack(push,1)
class udp_relayer :public tunnel{
private:
    bool dns=false,dst=false;
    int ip_hdr_len,hdr_len;
    ssize_t recv_len;
    char m_data[UDP_BUFFER_LEN];
public:
    udp_relayer(int epoll_fd,char* buf,u_int16_t total_len);
    virtual void on_epoll_in() override;

    virtual void on_epoll_out() override;

    virtual void connect_remote() override;


    virtual ~udp_relayer() override ;
};
#pragma pack(pop)

#endif
