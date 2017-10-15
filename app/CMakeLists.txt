cmake_minimum_required(VERSION 3.4.1)

add_library( core

             SHARED

             src/main/cpp/core.cpp
             src/main/cpp/ev_server/ev_server.cpp
             src/main/cpp/tunnel/tunnel.cpp
             src/main/cpp/utils/utils.cpp
             src/main/cpp/utils/global.cpp
             src/main/cpp/utils/nat_mgr.cpp
             src/main/cpp/utils/conf.cpp
             src/main/cpp/udp/udp_relayer.cpp
             src/main/cpp/udp/udp_ontcp.cpp
             src/main/cpp/tcp/tcp_base.cpp
             src/main/cpp/tcp/raw_tun.cpp
             src/main/cpp/tcp/cproxy.cpp
             src/main/cpp/hdr_utils/inet_ip4.cpp
             src/main/cpp/hdr_utils/inet_tcp.cpp
             src/main/cpp/hdr_utils/inet_udp.cpp)
find_library(log-lib

              log )
target_link_libraries(core

                       ${log-lib} )