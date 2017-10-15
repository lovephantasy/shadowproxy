//
// Created by lovefantasy on 2017/6/2.
//

#ifndef SHADOWPROXY_CPROXY_H
#define SHADOWPROXY_CPROXY_H

#include "tcp_base.h"

#include "../utils/nat_session.h"

#pragma pack(push,1)
class cproxy : public tcp_base {

public:
    cproxy(int, int);

    virtual bool determine_connect(void *pVoid) override;

    cproxy(int, sockaddr_in *, tcpsession *s);

    virtual void connect_remote() override;

    virtual ~cproxy() override ;

    int8_t req_type=0;
private:
    tcpsession *s = 0;
    u_int32_t other_len, header_len;
    bool m_is_first = true;
    char *header = 0, *other = 0, *method = 0, *url = 0, *uri = 0, *host = 0, *version = 0, *tmp = 0;
private:
    virtual int encrypt(const char* data,size_t data_len) override ;

    virtual int decrypt()  override;

    virtual void on_connect() override ;

    void send_connect();

    int is_connectec(const char* data);
    /*** 正则表达式字符串替换，str为可用free释放的指针 */
    char *reg_strrep(char *str, regex_t *src, char *dest);

    char *str_replace(char *str, const char *src, char *dest);

    /***将ip和端口用:拼接 */
    char *splice_ip_port(char *ip, uint16_t port);

    /*** 在请求头中获取host */
    char *get_host(char *header);

    /*** 删除请求头中的头域 */
    int8_t del_hdr(char *header, struct modify *head);

    int8_t parse_http_request(const char *request, size_t request_len);

    int8_t parse_connect_request(const char *request, size_t request_len  );
    /*** 修改请求头 返回值: -1为错误，0为需要代理的请求，1为不需要代理的请求*/
    int8_t modify_request(const char* data,size_t data_len);

    void free_http_request();

    u_int8_t get_type(const char *data);
};

#pragma pack(pop)


#endif //SHADOWPROXY_CPROXY_H
