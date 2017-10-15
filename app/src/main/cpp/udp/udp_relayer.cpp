//
// Created by lovefantasy on 2017/6/4.
//
#include "udp_relayer.h"
#include "../utils/utils.h"


udp_relayer::udp_relayer(int epoll_fd, char *buf, u_int16_t packet_total_len) : tunnel(epoll_fd, 0) {

    ip_hdr_len = inet_ip4::getHeaderLength(buf);

    hdr_len = ip_hdr_len + UDP_HDR_LEN;

    memcpy(m_data, buf, hdr_len);

    recv_len = packet_total_len - hdr_len;
}

void udp_relayer::connect_remote() {
    do {

        if ((m_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            LOGE("udp_proxy", "socket error: %s", strerror(errno));
            dst = true;
            break;
        }

        set_nonblock();

        if (!g_env->CallBooleanMethod(g_vpn_object, g_protect_socket, m_fd)) {
            LOGE("udp_proxy", "protect remote %d for %u failed,error: %s", m_fd,
                 ntohs(m_remote_addr->sin_port), strerror(errno));
            dst = true;
            break;
        }


        if (inet_udp::getDestinationPort(m_data) == htons(53)) {
            dns = true;
            m_remote_addr = &c->dns.dst;
        } else {
            m_remote_addr = (sockaddr_in *) malloc(sizeof(sockaddr_in));
            m_remote_addr->sin_family = AF_INET;
            m_remote_addr->sin_addr.s_addr = inet_ip4::getDestinationAddress(m_data);
            m_remote_addr->sin_port = inet_udp::getDestinationPort(m_data);
        }


        /**
         * 此处要注意指针的使用！！
         */
        connect(m_fd, (sockaddr *) m_remote_addr, sizeof(sockaddr));


        write(m_fd, m_data + hdr_len, (size_t) recv_len);

    } while (0);

    if (dst)
        delete this;
}

void udp_relayer::on_epoll_in() {
    do {
        if ((recv_len = read(m_fd, m_data + hdr_len, UDP_BUFFER_LEN - hdr_len)) == -1 ||
            recv_len == 0) {
            LOGE("udp_proxy", "recv error: %s", strerror(errno));
            break;
        }
        /**
         * 更新IP首部 目标地址，端口，源地址，端口，totallength,id,fragments
         */

        inet_ip4::setSourceAddress(m_data, inet_ip4::getDestinationAddress(m_data));
        inet_ip4::setDestinationAddress(m_data, vpn_addr);

        ///端口
        u_int16_t dst_port = inet_udp::getDestinationPort(m_data);
        inet_udp::setDestinationPort(m_data, inet_udp::getSourcePort(m_data));
        inet_udp::setSourcePort(m_data, dst_port);

        ///typeofservice
        *(m_data + 1) = (char) 0xD4;

        ///totallength
        *((u_int16_t *) m_data + 1) = htons(recv_len + hdr_len);

        ///id
        *((u_int16_t *) m_data + 2) = 0;

        ///fragments
        *((u_int16_t *) m_data + 3) = 0;

        /**
         * *更新UDP首部 totallength checksum
          */

        ///udp totallength
        inet_udp::setTotalLength(m_data, (u_int16_t) recv_len + (u_int16_t) UDP_HDR_LEN);


        ///updateudpchecksum
        utils::updateipchecksum(m_data);
        utils::updateudpchecksum(m_data);

        /**
         * 写入VPN fd
         */
        write(g_vpn_fd, m_data, recv_len + hdr_len);
    } while (0);

    delete this;
}


udp_relayer::~udp_relayer() {
    if (!dns) {
        free(m_remote_addr);
    }
}

void udp_relayer::on_epoll_out() {

}
