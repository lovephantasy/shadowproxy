//
// Created by lovefantasy on 2017/6/14.
//

#include <pthread.h>
#include <malloc.h>
#include "global.h"

u_int16_t tcp_port = htons(12306);

in_addr_t vpn_addr = inet_addr("10.0.0.86");

struct struct_tcpsession * d_session=0;

bool is_running = false;

 void thread_quit(int sig){
    __android_log_print(ANDROID_LOG_ERROR, "quit", "%d", sig);
    pthread_exit(NULL);
}

