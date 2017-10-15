//
// Created by lovefantasy on 17-6-23.
//

#ifndef SHADOWPROXY_PROXY_CONFIG_H
#define SHADOWPROXY_PROXY_CONFIG_H

#define HTTPDNS_REQUEST "GET /d?dn=[D] HTTP/1.0\r\nHost: [H]\r\n\r\n"

#include <regex.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <alloca.h>
#include "global.h"
#include <stdlib.h>

#pragma pack(push,1)
struct tcp_conf {
    struct sockaddr_in dst;
    struct modify *m;
};
#pragma pack(pop)

#pragma pack(push,1)
struct httpdns_conf {
    struct sockaddr_in dst;
    char *connect_req;
    char *http_req;
    char *cachePath;
    int connect_req_len;
    unsigned cacheLimit;
};
#pragma pack(pop)

#pragma pack(push,1)
class conf {
public:
    conf();
    ///httpdns部分
    httpdns_conf dns;
    ///http部分
     tcp_conf http;
    ///https部分
     tcp_conf https;
    ///global部分
   // int tcp_listen_fd, dns_listen_fd, uid, procs;
    u_int mode :3;
    u_int http_only_get_post :1;
    u_int strict_modify :1;

public:
    void read_conf(char const *path);

    /*** HTTPS模式的字符串提前修改 */
    char *ssl_req_replace(char *str);
    /*** 字符串预处理 */
    char *string_pretreatment(char *str);
    /***定位变量*/
    int8_t location_var_val(char *content, char **var, char **val_begin, char **val_end);

    /*** 在buff中读取模块内容 */
    char *read_module(char *buff,const char *module_name);

    /***解析global模块*/
    void parse_global_module(char *content);

    /*** 读取TCP模块 */
    int8_t parse_tcp_module(char *content, tcp_conf *tcp,int8_t https);

    /*** 读取HTTPDNS模块 */
    int8_t parse_httpdns_module(char *content);

    /*** 字符串替换，str为可以用free释放的指针 */
    char *str_replace(char *str, const char *src, const char *dest);

};
#pragma pack(pop)


#endif //SHADOWPROXY_PROXY_CONFIG_H
