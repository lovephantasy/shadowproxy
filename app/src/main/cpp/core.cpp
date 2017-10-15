#include <jni.h>
#include "utils/global.h"
#include "ev_server/ev_server.h"
#include "hdr_utils/inet_udp.h"
#include "hdr_utils/inet_tcp.h"
#include "hdr_utils/inet_ip4.h"

#define IP_PACKET_SIZE 32768
/***
     env与当前线程相关 jobject表示当前java对象可跨线程。
     jvm线程无关。使用AttachCurrentThread来得到当前线程env，线程结束时DetachCurrentThread;
*/

JavaVM *g_jvm;
JNIEnv *g_env;

/**
 *  通过vpnservice对象调用protect方法能保证兼容性。
 */
jclass g_vpn_class;
jobject g_vpn_object;
jmethodID g_protect_socket;
/**
 * tun设备fd
 */
jint g_vpn_fd;

///全局对象声明 必须要加class。。。。

class conf *c;


static pthread_t tcp_thread;
static pthread_t dns_thread;
static pthread_t main_thread;


static inet_ip4 *ipHeader;
static inet_tcp *tcpHeader;
static inet_udp *udpHeader;


class ev_server *cls_evserver;


/***
 * 初始化natmanager
 */
map<u_int16_t, tcpsession> *natmgr::tcp_sessions = new map<u_int16_t, tcpsession>;
map<u_int16_t, tcpsession> *natmgr::udp_sessions = new map<u_int16_t, tcpsession>;


extern "C"
void *start_tcp_server(void *);
extern "C"
void *start_udp_server(void *);
extern "C"
void *start_mainloop(void *);
extern "C"
void mainloop();
extern "C"
void on_packet_received(char *buffer, ssize_t len);

inline u_int8_t get_type(const char *data);

extern "C"
JNIEXPORT void JNICALL
Java_android_shadowproxy_ShadowProxy_initCore(JNIEnv *env, jobject instance) {
    LE("core", "jni init");

    env->GetJavaVM(&g_jvm);

    g_vpn_class = (jclass) env->NewGlobalRef(env->GetObjectClass(instance));

    g_vpn_object = env->NewGlobalRef(instance);

    if ((g_protect_socket = env->GetMethodID(g_vpn_class, "protect", "(I)Z")) == NULL)
        LE("core", "protect id is null");
}

/**
 * 启动VPN线程，dns，tcp服务器线程。
 */
extern "C"
JNIEXPORT void JNICALL
Java_android_shadowproxy_ShadowProxy_startVPN_1jni(JNIEnv *env, jobject, jint fd, jstring path_) {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGABRT, SIG_IGN);

    /*** STEP 1 read config file***/
    const char *path = env->GetStringUTFChars(path_, 0);

    // LOGE("core", "%s",path);

    c = new class conf();

    c->read_conf(path);

    env->ReleaseStringUTFChars(path_, path);

    /*** STEP 2 set running flag***/

    is_running = true;

    /*** STEP 3 get global vpn fd***/

    g_vpn_fd = fd;

    /*** STEP 4 set creat worker threads***/
    pthread_create(&tcp_thread, NULL, start_tcp_server, NULL);
    pthread_create(&main_thread, NULL, start_mainloop, NULL);

    /*** print vpn fd***/

    LOGE("core", "tun fd: %d", fd);


}


extern "C"
JNIEXPORT void JNICALL
Java_android_shadowproxy_ShadowProxy_stopVPN_1jni(JNIEnv *, jobject) {
    is_running = false;

    /**
     * 如何检查fd的有效性？
     */
    pthread_detach(tcp_thread);
    pthread_kill(tcp_thread, SIGQUIT);

}


extern "C"
void *start_tcp_server(void *) {
    signal(SIGQUIT, thread_quit);
    g_jvm->AttachCurrentThread(&g_env, NULL);
    cls_evserver = new class ev_server();
    cls_evserver->tcp_loop();
    g_jvm->DetachCurrentThread();
    LE("core", "tcp thread destory");
    return NULL;
}


extern "C"
void *start_mainloop(void *) {
    mainloop();
    LE("core", "main thread destory");
    return NULL;
}

extern "C"
void mainloop() {
    char buffer[IP_PACKET_SIZE];
    ipHeader = new inet_ip4(buffer);
    tcpHeader = new inet_tcp(buffer);
    udpHeader = new inet_udp(buffer);
    ssize_t readBytes;
    while (is_running) {
        if ((readBytes = read(g_vpn_fd, buffer, IP_PACKET_SIZE)) > 0) {
            on_packet_received(buffer, readBytes);
        } else {
            usleep(200);
        }
    }
    delete ipHeader;
    delete tcpHeader;
    delete udpHeader;
}

extern "C"
void on_packet_received(char *buffer, ssize_t len) {
    switch (ipHeader->getProtocol()) {
        case 6:
            if (ipHeader->getSourceAddress() == vpn_addr) {
                // LOGE("core", "tcp: %u:%d local: %u",ipHeader->getSourceAddress(),ntohs(tcpHeader->getSourcePort()),vpn_addr);
                tcpsession *session;
                if (tcpHeader->getSourcePort() == tcp_port) {
                    session = natmgr::get_tcp_session(tcpHeader->getDestinationPort());

                    ipHeader->setSourceAddress(ipHeader->getDestinationAddress());
                    tcpHeader->setSourcePort(session->remote_port);

                    ipHeader->setDestinationAddress(vpn_addr);

                    utils::updateipchecksum(ipHeader);
                    utils::updatetcpchecksum(ipHeader, tcpHeader);

                    write(g_vpn_fd, buffer, (size_t) len);

                } else {
                    session = natmgr::get_tcp_session(tcpHeader->getSourcePort());

                    if (session == NULL) {

                        session = natmgr::create_tcp_session(
                                ipHeader->getDestinationAddress(),
                                tcpHeader->getDestinationPort(),
                                tcpHeader->getSourcePort());

                    } else if (session->remote_ip != ipHeader->getDestinationAddress() ||
                               session->remote_port != tcpHeader->getDestinationPort()) {

                        natmgr::remove_tcp_session(tcpHeader->getSourcePort());

                        session = natmgr::create_tcp_session(
                                ipHeader->getDestinationAddress(),
                                tcpHeader->getDestinationPort(),
                                tcpHeader->getSourcePort());
                    }

                    /***坑： 位运算符优先级小于算数***/
                    int payloadsize =
                            ipHeader->getDataLength() - (((tcpHeader->getHeaderLength() << 2)));
                    session->packets_send++;
                    if (session->packets_send == 2 && payloadsize == 0) {
                        ///后续步骤会处理。
                        return;
                    }
                    if (session->bytes_send == 0 && payloadsize > 0 && c->mode != RAW) {
                        session->request_type = get_type(
                                ipHeader->data + (ipHeader->getHeaderLength() << 2) +
                                (tcpHeader->getHeaderLength() << 2));

                        /* in_addr t;
                         t.s_addr = ipHeader->getDestinationAddress();
                         if (session->request_type != HTTP_CONNECT) {
                             LOGE(data, char:  %04X %c, *(buffer + offset), *(buffer + offset));
                             LOGE(core, remote: %s:%d, inet_ntoa(t),
                                  ntohs(tcpHeader->getDestinationPort()));
                         }*/
                    }

                    u_int32_t tmp = ipHeader->getSourceAddress();
                    ipHeader->setSourceAddress(ipHeader->getDestinationAddress());


                    ipHeader->setDestinationAddress(tmp);
                    tcpHeader->setDestinationPort(tcp_port);

                    utils::updatetcpchecksum(ipHeader, tcpHeader);
                    write(g_vpn_fd, buffer, (size_t) len);
                    session->bytes_send += payloadsize;
                    /***RST FIN REMOVE cache***/
                }
                // if (d_session == session) {
                //natmgr::remove_tcp_session(session->source_port);
                //free(d_session);
                // d_session = 0;
                //}

            }
            break;
        case 17:
            // LOGE("core", "udp: %u:%d local: %u",ipHeader->getSourceAddress(),ntohs(udpHeader->getSourcePort()),vpn_addr);
            cls_evserver->on_udp_recv(buffer, len);
            break;
        default:
            break;
    }

}

inline u_int8_t get_type(const char *data) {
    switch (*data) {
        case 'G':
            return HTTP;
        case 'P':
            if (*(data + 1) == 'O')
                return HTTP;
            else
                return HTTP_OTHERS;///PATCH
        case 0x16:
            return SSL;
        case 'C':
            if (*(data + 2) == 'N')
                return HTTP_CONNECT;
            else
                return HTTP_OTHERS;///COPY
        case 'O':   ///OPTIONS
            return HTTP_OTHERS;
        case 'M':   ///MOVE
            return HTTP_OTHERS;
        case 'H':   ///HEAD
            return HTTP_OTHERS;
        case 'T':   ///TRACE
            return HTTP_OTHERS;
        case 'D':   ///DELETE
            return HTTP_OTHERS;
        case 'L':   ///LINK
            return HTTP_OTHERS;
        case 'U':   ///UNLINK
            return HTTP_OTHERS;
        case 'W':   ///WRAPPED
            return HTTP_OTHERS;
        default:
            return SSL;
    }
}



