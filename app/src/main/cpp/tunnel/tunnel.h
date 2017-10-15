//
// Created by lovefantasy on 17-8-13.
//

#ifndef SHADOWPROXY_TUNNEL_H
#define SHADOWPROXY_TUNNEL_H
#include <sys/epoll.h>
#include <linux/in.h>
#include <fcntl.h>

#pragma pack(push,1)
class tunnel {
protected:
    epoll_event m_ev;
    int m_fd, m_epoll_fd;
    /***client请求的真实地址***/
    sockaddr_in *m_remote_addr = 0;
    /***程序要连接的地址***/
    sockaddr_in *m_proxy_addr = 0;
public:
    tunnel *m_pair;
    /***是否已经连接***/
    bool m_is_connected = false;

    tunnel(int, int);

    tunnel(int epoll_fd, sockaddr_in *dst);

    virtual ~tunnel();

    virtual void connect_remote();

    virtual void regist_read();
    virtual void cancel_read();

    virtual void write_data(const char* data,size_t data_len);
    virtual void on_epoll_in()=0;

    virtual void on_epoll_out()=0;
    virtual void close_connection(bool);

protected:
    int set_nonblock() {
        int o_opts ;
        if((o_opts= fcntl(m_fd, F_GETFL))==-1){
            o_opts=0;
        }
        return fcntl(m_fd, F_SETFL, o_opts | O_NONBLOCK);
    };


};

#pragma pack(pop)

#endif //SHADOWPROXY_TUNNEL_H
