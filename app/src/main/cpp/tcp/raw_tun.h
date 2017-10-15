//
// Created by lovefantasy on 2017/6/2.
//

#ifndef SHADOWPROXY_RAWTUN_H
#define SHADOWPROXY_RAWTUN_H

#include "tcp_base.h"
#pragma pack(push,1)
class raw_tun : public tcp_base {
public:

    raw_tun(int, int);

    raw_tun( int, sockaddr_in *);

    virtual bool determine_connect(void *pVoid) override;

    virtual void connect_remote() override;

    virtual ~raw_tun() override;

private:
    virtual int encrypt(const char* data,size_t data_len) override;

    virtual int decrypt() override;

    virtual void on_connect() override;
};
#pragma pack(pop)


#endif //SHADOWPROXY_RAWTUNNEL_H
