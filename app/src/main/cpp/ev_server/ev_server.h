//
// Created by lovefantasy on 2017/6/2.
//

#ifndef SHADOWPROXY_TCPSERVER_H
#define SHADOWPROXY_TCPSERVER_H


#include <linux/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "../utils/utils.h"
#include "../utils/nat_mgr.h"
#include "../tcp/raw_tun.h"
#include "../tcp/cproxy.h"
#include "../utils/global.h"
#include "../tunnel/tunnel.h"
#include "../udp/udp_relayer.h"

#define TCP_MAX_FD 256

#pragma pack(push,1)
class ev_server {
private:
    int tcp_sfd, epoll_fd;
    sockaddr_in m_addr;
    sockaddr_in m_client_addr, *m_tmp_addr;
    socklen_t m_client_addr_len;
    epoll_event evs[TCP_MAX_FD], ev;
    cproxy *remote;
    raw_tun *client;
    int set_nonblock(int fd) {
        int o_opts ;
        if((o_opts= fcntl(fd, F_GETFL))==-1){
            o_opts=0;
        }
        return fcntl(fd, F_SETFL, o_opts | O_NONBLOCK);
    };
public:

    tunnel *d_tunnel = 0;

    ev_server();

    ~ev_server();

    void tcp_loop();

    void on_accept();

    void on_udp_recv(char* buf,int total_len);


};
#pragma pack(pop)


#endif //SHADOWPROXY_TCPSERVER_H
