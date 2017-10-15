//
// Created by lovefantasy on 2017/6/1.
//
/***
 * Tunnel基类,所有协议的实现必须继承此类，
 *         此类实现透明代理。
 */
#ifndef SHADOWPROXY_TCP_BASE_H
#define SHADOWPROXY_TCP_BASE_H

//#define NODELAY

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdlib.h>
#include <jni.h>

#include "../tunnel/tunnel.h"
#pragma pack(push,1)
class tcp_base : public tunnel {

protected:

    /***是否开始向pair写数据***/
    bool m_begin_write_pair = false;

    char *m_recv_data = 0,*m_ready_data = 0;

    ssize_t m_read_len, m_writed_len, m_ready_len;
    /***接收缓冲区长度***/
    size_t m_recv_data_len,m_recv_buf_len=0;
private:

    /***临时缓冲区***/
    char *tmp;
    /***剩余缓冲区,缓冲区长度***/
    char *m_remain_data = 0;
    /***是否还有未写完的数据***/
    ssize_t m_remain_len=0;

private:

    void on_writeable();


public:

    tcp_base(int, int);

    tcp_base(int, sockaddr_in *);

    virtual ~tcp_base() override;

    virtual void on_epoll_in() override;

    virtual void on_epoll_out() override;

    virtual bool determine_connect(void *)=0;

    virtual void connect_remote() override;

    virtual void regist_read() override;

    virtual void cancel_read() override;

    virtual void close_connection(bool b) override;

    virtual void write_data(const char *data, size_t data_len) override;

protected:

    void begin_contact();

    virtual int encrypt(const char* data,size_t data_len)=0;

    virtual int decrypt()=0;

    virtual void on_connect()=0;

};

#pragma pack(pop)


#endif
