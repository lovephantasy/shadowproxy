//
// Created by lovefantasy on 17-8-13.
//

#include "tunnel.h"

tunnel::tunnel(int e_fd, int fd) {
    m_fd=fd;
    m_epoll_fd=e_fd;
}

tunnel::tunnel(int epoll_fd, sockaddr_in *dst) {
    m_epoll_fd=epoll_fd;
    m_remote_addr=dst;
}

void tunnel::connect_remote() {

}

void tunnel::regist_read() {

}

void tunnel::cancel_read() {

}

void tunnel::close_connection(bool) {

}

void tunnel::write_data(const char *data, size_t data_len) {

}

tunnel::~tunnel() {

}


