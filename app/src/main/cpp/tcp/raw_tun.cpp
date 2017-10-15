//
// Created by lovefantasy on 2017/6/2.
//

#include "raw_tun.h"


raw_tun::raw_tun( int tcp_fd, int fd) : tcp_base( tcp_fd, fd) {

}

raw_tun::raw_tun(int tcp_fd, sockaddr_in *dst) : tcp_base(tcp_fd, dst) {

}


void raw_tun::on_connect() {
    m_begin_write_pair=true;
    begin_contact();
}

//

int raw_tun::encrypt(const char* data,size_t data_len) {
    m_ready_data=(char*)data;
    m_ready_len=data_len;
    return 0;
}

int raw_tun::decrypt() {
    if (!m_begin_write_pair) {
        m_begin_write_pair=true;
    }
    return 0;
}

void raw_tun::connect_remote(){
    m_proxy_addr=m_remote_addr;
}

raw_tun::~raw_tun() {


}


bool raw_tun::determine_connect(void *pVoid) {
    return true;
}


