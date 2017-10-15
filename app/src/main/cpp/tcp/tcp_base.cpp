//
// Created by lovefantasy on 2017/6/1.
//

#include "tcp_base.h"
#include "../ev_server/ev_server.h"

tcp_base::tcp_base(int epoll_fd, int fd):tunnel(epoll_fd,fd) {
    set_nonblock();
}

tcp_base::tcp_base(int epoll_fd, sockaddr_in *dst):tunnel(epoll_fd,dst) {
}

/****
 * 开始连接
 */

void tcp_base::connect_remote() {
    int opt = 1;
    if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOGE("tcp_base", "socket error: %s", strerror(errno));
        close_connection(true);
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
        close_connection(true);
        return;
    }

    /***regist epoll event***/
    m_ev.data.ptr = this;
    m_ev.events = EPOLLOUT | EPOLLET;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_fd, &m_ev);
    int r = connect(m_fd, (sockaddr *) m_proxy_addr, sizeof(sockaddr));
    if (r == -1 && errno != EINPROGRESS) {
        close_connection(true);
        return;
    } else {
        m_is_connected = true;
        m_pair->m_is_connected = true;
    }

}

void tcp_base::regist_read() {

    /**
     * 设置pair tunnnel 的事件(初始化时pair的ev没有设置值)
     */
    if (m_ev.data.ptr == this) {
        m_ev.events = EPOLLIN | EPOLLET;
        epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, m_fd, &m_ev);

    } else {
        m_ev.data.ptr = this;
        m_ev.events = EPOLLIN | EPOLLET;
        epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_fd, &m_ev);
    }
}

void tcp_base::begin_contact() {
    this->regist_read();
    m_pair->regist_read();

}

/****
 *  epoll_wait -> EPOLLOUT
 */

void tcp_base::on_epoll_out() {
    if (!m_begin_write_pair) {
        on_connect();
    } else {
        on_writeable();
    }
}

/****
 * epoll_wait -> EPOLLIN
 */
void tcp_base::on_epoll_in() {
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

void tcp_base::cancel_read() {
    m_ev.events = EPOLLET;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, m_fd, &m_ev);
}


/****
 *    tunel写数据只调用这个函数。
 *    如果数据没写完，则pair_tunnel也不能接收epoll_in事件。。即使接收了。。强行返回。。
 *    写完之后重新注册写事件即可再次读取上次因返回未读的数据。
 */
void tcp_base::write_data(const char *data, size_t data_len) {
    if (!m_remain_len) {
        if (encrypt(data, data_len)) {
            close_connection(true);
            return;
        }
    } else {
        m_ready_data = (char *) data;
        m_ready_len = data_len;
    }
    if ((m_writed_len = write(m_fd, m_ready_data, (size_t) m_ready_len)) == m_ready_len) {
        /***handle the last time remain data***/
        if (m_remain_len) {
            free(m_remain_data);
            m_remain_data = 0;
            begin_contact();
            m_remain_len = 0;
        }
    } else {
        if (m_writed_len > 0) {
            if (!m_remain_len) {
                m_remain_len = m_ready_len - m_writed_len;
                if ((m_remain_data = (char *) malloc((size_t) m_remain_len)) == NULL) {
                    if (m_ready_data != data) {
                        free(m_ready_data);
                        m_ready_data = 0;
                    }
                    close_connection(true);
                    return;
                }
                memcpy(m_remain_data, m_ready_data + m_writed_len, (size_t) m_remain_len);
                m_pair->cancel_read();

                m_ev.events = EPOLLOUT | EPOLLET;
                if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, m_fd, &m_ev) < 0) {
                    LOGE("tcp_base", "begin_recv2 error: %s", strerror(errno));
                    free(m_remain_data);
                    m_remain_data = 0;
                    close_connection(true);
                    return;
                }
            } else {
                m_remain_len = m_ready_len - m_writed_len;
                char *tmp;
                if ((tmp = (char *) malloc((size_t) m_remain_len)) == NULL) {
                    close_connection(true);
                    return;
                }
                memcpy(tmp, m_ready_data + m_writed_len, (size_t) m_remain_len);
                free(m_remain_data);
                m_remain_data = tmp;
            }

        } else if (m_writed_len == -1 && errno != EAGAIN) {
            LOGE("write_data", "%s", strerror(errno));
            if (m_ready_data != data) {
                free(m_ready_data);
                m_ready_data = 0;
            }
            close_connection(true);
            return;
        }
    }
    /***free the memory allwed last time***/
    if (m_ready_data != data) {
        free(m_ready_data);
        m_ready_data = 0;
    }

}


/****
 * EPOLLOUT 可写，区分是否已经连接
 */

void tcp_base::on_writeable() {
    if (m_remain_len) {
        write_data(m_remain_data, (size_t) m_remain_len);
    }
}


void tcp_base::close_connection(bool do_pair) {
    close(m_fd);
    /***del epoll event***/
    m_ev.data.ptr = this;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, m_fd, &m_ev);
    /***ssssssssssssssssssssssssssssssssssss*/
    m_ev.data.ptr = 0;

    if (do_pair) {
        /***tell server***/
        cls_evserver->d_tunnel = this;
        /***dispose pair***/
        m_pair->close_connection(false);
    }
}



tcp_base::~tcp_base() {
    if (m_remote_addr) {
        free(m_remote_addr);
    }

    if (m_recv_data) {
        free(m_recv_data);
    }
    if (m_remain_data) {
        free(m_remain_data);
    }

}













