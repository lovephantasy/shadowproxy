//
// Created by lovefantasy on 2017/6/4.
//

#include <unistd.h>
#include "udp_ontcp.h"

udp_ontcp::udp_ontcp(int epoll_fd, sockaddr_in *dst) : tunnel(epoll_fd, dst) {

}


void udp_ontcp::on_epoll_in() {
    /***
     * 阻塞模式下，read已经关闭的socket会返回0,返回-1代表出错。
     *
     * 非阻塞模式下，返回0代表读到文件结尾(没读到数据)。返回-1并且errno为EAGAIN表示读完，errno为其他代表出错。
     *
     * 已解决的问题：若pair->fd的缓冲区较小，导致this->fd缓冲区没有读完。ET模式将不再响应事件。
     * 解决： 在pair->fd写完后，重新注册this->fd的EPOLLIN事件
     */

    /**如果pair_tunnel有数据没写完则取消本次读取事件**/
    if (!m_is_connected) {
        m_pair->connect_remote();
        return;
    }

    /**重新分配，并全部读取完毕**/

    if (m_recv_data == NULL) {
        m_recv_data = (char *) malloc(BUFF_SIZE);
        m_recv_buf_len = BUFF_SIZE;
    }
    m_recv_data_len = 0;
    while ((m_read_len = read(m_fd, m_recv_data + m_recv_data_len,
                              m_recv_buf_len - m_recv_data_len)) > 0) {
        m_recv_data_len += m_read_len;
        if (m_recv_data_len < m_recv_buf_len) {
            break;
        } else {
            m_recv_buf_len += BUFF_SIZE;
            if ((tmp = (char *) realloc(m_recv_data, m_recv_buf_len)) == NULL) {
                LOGE("tcp_base", "epoll_in: %s", strerror(errno));
                close_connection(true);
                return;
            }
            m_recv_data = tmp;
        }
    }
    /**没读到数据,或者读取出错**/
    if (m_read_len == 0) {
        LOGE("on_readable1", "%s", strerror(errno));
        close_connection(true);
        return;
    } else if (m_read_len == -1 && errno != EAGAIN) {
        close_connection(true);
        return;
    }

    *(m_recv_data + m_recv_data_len) = '\0';

    if (decrypt()) {
        close_connection(true);
        return;
    }

    /**对已经建立连接的tunnel进行写操作**/
    if (m_begin_write_pair) {
        m_pair->write_data(m_recv_data, m_recv_data_len);
    }

}

void udp_ontcp::on_epoll_out() {

}

void udp_ontcp::connect_remote() {
    int opt = 1;
    if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOGE("tcp_base", "socket error: %s", strerror(errno));
        close_connection();
        return;
    }

    /***设置非阻塞模式***/
    set_nonblock();

#ifdef NODELAY
    setsockopt(m_fd, SOL_TCP, TCP_NODELAY, &opt, sizeof(opt));
#endif
#ifdef SO_NOSIGPIPE
    setsockopt(listen_sock, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt));
#endif
    /***protect socket fd***/
    if (!g_env->CallBooleanMethod(g_vpn_object, g_protect_socket, m_fd)) {
        LOGE("tcp_base", "protect remote % d for %u failed error: %s", m_fd,
             ntohs(m_proxy_addr->sin_port), strerror(errno));
        close_connection();
        return;
    }

    /***regist epoll event***/
    m_ev.data.ptr = this;
    m_ev.events = EPOLLOUT | EPOLLET;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_fd, &m_ev);
    int r = connect(m_fd, (sockaddr *) m_proxy_addr, sizeof(sockaddr));
    if (r == -1 && errno != EINPROGRESS) {
        close_connection();
        return;
    } else {
        m_is_connected = true;
    }

}

int udp_ontcp::send_udp_packet(char *buf, int total_len) {
    return 0;
}

void udp_ontcp::close_connection() {
    close(m_fd);
    /***del epoll event***/
    m_ev.data.ptr = this;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, m_fd, &m_ev);
    /***ssssssssssssssssssssssssssssssssssss*/
    m_ev.data.ptr = 0;
}
udp_ontcp::~udp_ontcp() {

}


