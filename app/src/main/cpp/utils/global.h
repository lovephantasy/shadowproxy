//
// Created by lovefantasy on 2017/6/14.
//

#ifndef SHADOWPROXY_GLOBAL_H
#define SHADOWPROXY_GLOBAL_H

#include <sys/types.h>
#include <android/log.h>
#include <jni.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex.h>
#include <limits.h>
#include "conf.h"
#include "nat_session.h"

#define LOGE(tag,format,...) do {__android_log_print(ANDROID_LOG_ERROR, tag,format,__VA_ARGS__);}while(0)
#define LE(tag,text) do{__android_log_print(ANDROID_LOG_ERROR, tag,#text);}while(0)

#define LOGD(tag,format,...) __android_log_print(ANDROID_LOG_DEBUG, #tag,#format,__VA_ARGS__);
#define LD(tag,text) __android_log_print(ANDROID_LOG_DEBUG, #tag,#text);

#define BUFF_SIZE 2048//4096//5120//8192  //16384
#define UDP_HDR_LEN 8
#define UDP_BUFFER_LEN 1500



/*** 请求类型 */
#define HTTP 2
#define HTTP_OTHERS 3
#define HTTP_CONNECT 4
#define SSL 5
#define OTHER 5
/** 请求头修改操作 */
#define SET_FIRST 1
#define DEL_HDR 2
#define REGREP 3
#define STRREP 4
/** 处理TCP请求模式 */
#define WAP 1
#define WAP_CONNECT 2
#define NET_CONNECT 3
#define NET_PROXY 4
#define NET 5
#define RAW 6
/** JNI global**/
extern JavaVM *g_jvm;
extern JNIEnv *g_env;
extern jobject g_vpn_object;
extern jmethodID g_protect_socket;
extern jint g_vpn_fd;
/** Server global**/
extern bool is_running;
extern u_int16_t tcp_port;
extern u_int16_t udp_port;
extern in_addr_t vpn_addr;

/** Proxy Config global**/
extern class conf *c;

extern struct struct_tcpsession * d_session;

extern class ev_server *cls_evserver;

/** Global call**/
extern void thread_quit(int sig);


#pragma pack(push,1)
struct modify {
    char *str;
    char *dest;
    struct modify *next;
    regex_t reg_src;
    unsigned flag :3; //判断修改请求头的操作
};
#pragma pack(pop)

#endif //SHADOWPROXY_GLOBAL_H
