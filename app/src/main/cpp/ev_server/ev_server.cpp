//
// Created by lovefantasy on 2017/6/2.
//

#include "ev_server.h"


ev_server::ev_server() {


    if ((tcp_sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        LOGE("tcperror", " %s", strerror(errno));
        exit(1);
    }

    int op = 1;
    if (setsockopt(tcp_sfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0) {
        LOGE("tcperror", " %s", strerror(errno));
        exit(1);
    }


    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_addr.sin_port = tcp_port;

    if (bind(tcp_sfd, (sockaddr *) &m_addr, sizeof(m_addr)) < 0) {
        LOGE("tcperror", " %s", strerror(errno));
        tcp_port = htons(11111);
        m_addr.sin_port = tcp_port;
        if (bind(tcp_sfd, (sockaddr *) &m_addr, sizeof(m_addr)) < 0) {
            LOGE("tcperror", " %s", strerror(errno));
            exit(1);
        }
    }
    if (listen(tcp_sfd, 512) < 0) {
        LOGE("tcperror", " %s", strerror(errno));
        exit(1);
    }

    if ((epoll_fd = epoll_create(TCP_MAX_FD)) < 0) {
        LOGE("tcperror", "%s", strerror(errno));
        exit(1);

    }
    set_nonblock(tcp_sfd);
    LOGE("tcp socket fd:", " %d", tcp_sfd);

    LOGE("tcp epoll_create:", "%d", epoll_fd);

    ///accept LT模式。
    ev.events = EPOLLIN; //| EPOLLET;
    ev.data.fd = tcp_sfd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_sfd, &ev);

}

void ev_server::tcp_loop() {
    int n, i, j;
    while (is_running) {
        n = epoll_wait(epoll_fd, evs, TCP_MAX_FD, -1);
        if (n < 0) {
            LOGE("tcpwaiterror", "epoll_fd: %d error: %s", epoll_fd, strerror(errno));
            continue;
        }
        for (i = 0; i < n; i++) {
            if (evs[i].data.fd == tcp_sfd) {
                on_accept();
            } else if (evs[i].events & EPOLLIN) {
                if (evs[i].data.ptr)
                    ((tunnel *) (evs[i].data.ptr))->on_epoll_in();
            } else if (evs[i].events & EPOLLOUT) {
                if (evs[i].data.ptr)
                    ((tunnel *) (evs[i].data.ptr))->on_epoll_out();
            }
            if (d_tunnel) {
                for (j = i + 1; j < n; j++) {
                    if (evs[j].data.ptr == d_tunnel || evs[j].data.ptr == d_tunnel->m_pair) {
                        /***屏蔽所有后续通道***/
                        evs[j].events = 0;
                    }
                }
                /***安全地销毁对象***/
                delete d_tunnel->m_pair;
                delete d_tunnel;
                d_tunnel = 0;
            }
        }
    }

    /***shutdown vpn****/
    close(tcp_sfd);
    close(epoll_fd);
}

/***
 *  accept LT模式版本
 */

void ev_server::on_accept() {
    m_client_addr_len = sizeof(sockaddr);
    int fd = accept(tcp_sfd, (sockaddr *) &m_client_addr, &m_client_addr_len);
    if (fd == -1) {
        LOGE("ev_server", "error: %s", strerror(errno));
        return;

    }
    tcpsession *s = natmgr::get_tcp_session(m_client_addr.sin_port);
    if (s == NULL) {
        LE("ev_server", "accept butnosession");
        return;

    }
    /**真实远程地址**/
    m_tmp_addr = (sockaddr_in *) malloc(sizeof(sockaddr_in));
    m_tmp_addr->sin_port = s->remote_port;
    m_tmp_addr->sin_family = AF_INET;
    m_tmp_addr->sin_addr.s_addr = s->remote_ip;

    remote = new cproxy(epoll_fd, m_tmp_addr, s);
    client = new raw_tun(epoll_fd, fd);

    client->m_pair = remote;
    remote->m_pair = client;

    /**
     *  开始连接服务器
     */
    if (remote->determine_connect(0)) {
        remote->connect_remote();
    } else {
        client->regist_read();
    }

}

void ev_server::on_udp_recv(char *buf, int total_len) {
    udp_relayer *udp = new udp_relayer(epoll_fd, buf, total_len);
    udp->connect_remote();
}

ev_server::~ev_server() {

}