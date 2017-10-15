//
// Created by lovefantasy on 17-6-23.
//


#include <errno.h>
#include "conf.h"
conf::conf() {

}

void conf::read_conf(char const *path) {
    __android_log_print(ANDROID_LOG_ERROR, "conf", "%s",path);

    char *buff, *global, *http, *https, *httpdns;
    FILE *file;
    long file_size;

    /* 读取配置文件到缓冲区 */
    file = fopen(path, "r");
    if (file == NULL) {
        perror("cannot open conf file.");
        __android_log_print(ANDROID_LOG_ERROR, "conf", "%s", strerror(errno));

    }
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    buff = (char *) alloca(file_size + 1);
    if (buff == NULL) {
        perror("out of memory.");
    }
    rewind(file);
    fread(buff, file_size, 1, file);
    fclose(file);
    buff[file_size] = '\0';

    /* 读取global模块内容 */
    if ((global = read_module(buff, "global")) == NULL) {
        perror("wrong conf file or out of memory.");
        __android_log_print(ANDROID_LOG_ERROR, "conf", "%s", strerror(errno));
    }
    parse_global_module(global);
    free(global);

    /* 读取http模块内容 */
    if (((http = read_module(buff, "http")) == NULL) ||
        parse_tcp_module(http, &this->http, 0) != 0) {
        free(http);
        perror("wrong conf file or out of memory.");
        __android_log_print(ANDROID_LOG_ERROR, "conf", "%s", strerror(errno));
    }

    /* 读取https模块 */
    if (((https = read_module(buff, "https")) == NULL) ||
        parse_tcp_module(https, &this->https, 1) != 0) {
        perror("wrong conf file or out of memory.");
        free(https);
        __android_log_print(ANDROID_LOG_ERROR, "conf", "%s", strerror(errno));
    }

    ///构建模式CONNECT请求
    /**conn_t ssl;
    memset(&ssl, 0, sizeof(conn_t));
    ssl.reqType = HTTP_CONNECT;
    ssl.host = (char *) 1; //不保存Host
    if (modify_request(strdup(CONNECT_HEADER), strlen(CONNECT_HEADER), &ssl) != 0)
        perror("out of memory.");
    def_ssl_req = ssl.ready_data;*/


    /* 读取httpdns模块 */
    if ((httpdns = read_module(buff, "udp_ontcp")) == NULL || parse_httpdns_module(httpdns) != 0) {
        free(httpdns);
        perror("wrong conf file or out of memory.");
        __android_log_print(ANDROID_LOG_ERROR, "conf", "%s", strerror(errno));
    }

}

char *conf::ssl_req_replace(char *str) {
    str = str_replace(str, "[M]", "CONNECT");
    str = str_replace(str, "[V]", "HTTP/1.1");
    str_replace(str, "[U]", "/");
    return str_replace(str, "[url]", "[H]");
}

char *conf::string_pretreatment(char *str) {
//删除换行和缩进
    char *lf, *p;
    while ((lf = strchr(str, '\n')) != NULL) {
        for (p = lf + 1; *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'; p++);
        strcpy(lf, p);
    }
    str_replace(str, "\r", "");  //Windows换行是\r\n

    //替换转义字符
    str_replace(str, "\\b", "\b");
    str_replace(str, "\\v", "\v");
    str_replace(str, "\\f", "\f");
    str_replace(str, "\\a", "\a");
    str_replace(str, "\\t", "\t");
    str_replace(str, "\\r", "\r");
    return str_replace(str, "\\n", "\n");
}

int8_t conf::location_var_val(char *content, char **var, char **val_begin, char **val_end) {
    char *p, *pn;

    while (1) {
        if (content == NULL)
            return 1;

        for (; *content == ' ' || *content == '\t' || *content == '\r' ||
               *content == '\n'; content++);
        if (*content == '\0')
            return 1;
        *var = content;
        pn = strchr(content, '\n');
        p = strchr(content, '=');
        if (p == NULL) {
            if (pn) {
                content = pn + 1;
                continue;
            } else
                return 1;
        }
        content = p;
        //将变量以\0结束
        for (p--; *p == ' ' || *p == '\t'; p--);
        *(p + 1) = '\0';
        //值的首地址
        for (content++; *content == ' ' || *content == '\t'; content++);
        if (*content == '\0')
            return 1;
        //双引号引起来的值支持换行
        if (*content == '"') {
            *val_begin = content + 1;
            *val_end = strstr(*val_begin, "\";");
            if (*val_end != NULL)
                break;
        } else
            *val_begin = content;
        *val_end = strchr(content, ';');
        if (*val_end == NULL)
            return 1;
        if (pn && *val_end > pn) {
            content = pn + 1;
            continue;
        }
        break;
    }

    *(*val_end)++ = '\0';
    string_pretreatment(*val_begin);
    //printf("var[%s]\nbegin[%s]\n\n", *var, *val_begin);
    return 0;
}

char *conf::read_module(char *buff,const char *module_name) {
    size_t len;
    char *p, *p0;

    len = strlen(module_name);
    p = buff;
    while (1) {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        if (strncasecmp(p, module_name, len) == 0) {
            p += len;
            while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
                p++;
            if (*p == '{')
                break;
        }
        if ((p = strchr(p, '\n')) == NULL)
            return NULL;
    }
    if ((p0 = strchr(++p, '}')) == NULL)
        return NULL;

    //printf("%s\n%s", module_name, content);
    return strndup(p, p0 - p);
}

void conf::parse_global_module(char *content) {
    char *var, *val_begin, *val_end, *p;

    while (location_var_val(content, &var, &val_begin, &val_end) == 0) {
        if (strcasecmp(var, "mode") == 0) {
            if (strcasecmp(val_begin, "wap") == 0)
                this->mode = WAP;
            else if (strcasecmp(val_begin, "net") == 0)
                this->mode = NET;
            else if (strcasecmp(val_begin, "wap_connect") == 0)
                this->mode = WAP_CONNECT;
            else if (strcasecmp(val_begin, "raw") == 0)
                this->mode = RAW;
            /*else if (strcasecmp(val_begin, "net_connect") == 0)
                this->mode = NET_CONNECT;
            else if (strcasecmp(val_begin, "net_proxy") == 0)
                this->mode = NET_PROXY;
            else if (strcasecmp(val_begin, "raw") == 0)
                this->mode = RAW;*/

        } /*else if (strcasecmp(var, "uid") == 0) {
            this->uid = atoi(val_begin);
        } else if (strcasecmp(var, "procs") == 0) {
            //this->procs = atol(val_begin);
        } else if (strcasecmp(var, "tcp_listen") == 0l) {
            if ((p = strchr(val_begin, ':')) != NULL && p - val_begin <= 15) {
                *p = '\0';
                ///this->tcp_listen_fd = tcp_listen(val_begin, atoi(p + 1));
            } else {
                ///this->tcp_listen_fd = tcp_listen("0.0.0.0", atoi(val_begin));
            }
        } else if (strcasecmp(var, "dns_listen") == 0) {
            if ((p = strchr(val_begin, ':')) != NULL && p - val_begin <= 15) {
                *p = '\0';
                /// this->dns_listen_fd = udp_listen(val_begin, atoi(p + 1));
            } else {
                // this->dns_listen_fd = udp_listen("127.0.0.1", atoi(val_begin));

            }
        } */else if (strcasecmp(var, "strict") == 0 && strcasecmp(val_begin, "on") == 0) {
            this->strict_modify = 1;
            __android_log_print(ANDROID_LOG_ERROR, "conf", "%d", this->mode);

        }

        content = strchr(val_end, '\n');
    }
}

int8_t conf::parse_tcp_module(char *content, tcp_conf *tcp, int8_t https) {
    struct modify *m, *m_save;
   // struct ssl_string *s;
    char *var, *val_begin, *val_end, *p, *src_end, *dest_begin;

    m = NULL;
   // s = ssl_str;
    tcp->dst.sin_family=AF_INET;
    while (location_var_val(content, &var, &val_begin, &val_end) == 0) {
        if (strcasecmp(var, "addr") == 0) {
            if ((p = strchr(val_begin, ':')) != NULL && p - val_begin <= 15) {
                *p = '\0';
                tcp->dst.sin_addr.s_addr = inet_addr(val_begin);
                tcp->dst.sin_port = htons(atoi(p + 1));
            } else {
                tcp->dst.sin_addr.s_addr = inet_addr(val_begin);
                tcp->dst.sin_port = htons(80);
            }
            goto next_line;
        }

        /* 以下判断为链表操作 */
        m_save = m; //保存前一个结构体指针
        if (m)
            m = m->next = (struct modify *) malloc(sizeof(*m));
        else
            tcp->m = m = (struct modify *) malloc(sizeof(*m));
        if (m == NULL)
            return 1;
        memset(m, 0, sizeof(*m));
        if (strcasecmp(var, "del_hdr") == 0) {
            m->flag = DEL_HDR;
            m->str = strdup(val_begin);
        } else if (strcasecmp(var, "set_first") == 0) {
            m->str = strdup(val_begin);
            //https模块首先替换部分字符串
            if (https)
                m->str = ssl_req_replace(m->str);
            if (m->str == NULL)
                return 1;
            m->flag = SET_FIRST;
        } else if (strcasecmp(var, "strrep") == 0 || strcasecmp(var, "regrep") == 0) {
            //定位 [源字符串结束地址] 和 [目标字符串首地址]
            p = strstr(val_begin, "->");
            if (p == NULL)
                return 1;
            for (src_end = p - 1; *src_end != '"'; src_end--) {
                if (src_end == val_begin)
                    return 1;
            }
            for (dest_begin = p + 2; *dest_begin != '"'; dest_begin++) {
                if (dest_begin == val_end)
                    return 1;
            }
            //复制原字符串
            m->str = strndup(val_begin, src_end - val_begin);
            //复制目标字符串
            if (val_end - dest_begin - 1 <= 0) //如果目标字符串为空
                m->dest = (char *) calloc(1, 1);
            else
                m->dest = strdup(dest_begin + 1);
            if (https) {
                m->str = ssl_req_replace(m->str);
                m->dest = ssl_req_replace(m->dest);
            }
            if (m->str == NULL || m->dest == NULL)
                return 1;
            if (*var == 's') //如果是普通字符串替换
                m->flag = STRREP;
            else //正则表达式字符串替换
            {
                m->flag = REGREP;
                regcomp(&m->reg_src, m->str, REG_NEWLINE | REG_ICASE | REG_EXTENDED);
                free(m->str);
            }
        } else if (https == 0) {
            if (strcasecmp(var, "only_get_post") == 0 && strcasecmp(val_begin, "on") == 0) {
                this->http_only_get_post = 1;
            } /**else if (strcasecmp(var, "proxy_https_string") == 0) {
                if (s == NULL)
                    ssl_str = s = (struct ssl_string *) malloc(sizeof(*s));
                else
                    s = s->next = (struct ssl_string *) malloc(sizeof(*s));
                if (s == NULL)
                    return 1;
                s->str = strdup(val_begin);
                if (s->str == NULL)
                    return 1;
                s->next = NULL;
            }*/
        }
        if (m->flag == 0) {
            free(m);
            if (m_save) {
                m = m_save;
                m->next = NULL;
            } else
                tcp->m = m = NULL;
        }

        next_line:
        content = strchr(val_end, '\n');
    }
    return 0;
}

int8_t conf::parse_httpdns_module(char *content) {
    char *var, *val_begin, *val_end, *p, *ip;
    uint16_t port;

    ip = NULL;
    port = 0;
    while (location_var_val(content, &var, &val_begin, &val_end) == 0) {
        if (strcasecmp(var, "addr") == 0) {
            if ((p = strchr(val_begin, ':')) != NULL && p - val_begin <= 15) {
                ip = val_begin;
                *p = '\0';
                this->dns.dst.sin_addr.s_addr = inet_addr(val_begin);
                port = atoi(p + 1);
            } else {
                ip = val_begin;
                port = 80;
            }
            this->dns.dst.sin_addr.s_addr = inet_addr(ip);
            this->dns.dst.sin_port = htons(port);
        } else if (strcasecmp(var, "http_req") == 0) {
            this->dns.http_req = strdup(val_begin);
            if (this->dns.http_req == NULL)
                return 1;
        } else if (strcasecmp(var, "cachePath") == 0) {
            this->dns.cachePath = strdup(val_begin);
           /** if (this->dns.cachePath != NULL) {
                if (read_cache_file() != 0)
                    return 1;
            }*/
        } else if (strcasecmp(var, "cacheLimit") == 0) {
            this->dns.cacheLimit = atoi(val_begin);
        }

        content = strchr(val_end, '\n');
    }

    if (ip == NULL || port == 0){
        perror("dns module no 'addr'.");  ///出错
        return 1;
    }
    //构建请求头格式
  /**  char dest[22];
    sprintf(dest, "%s:%d", ip, port);
    if (this->dns.http_req == NULL) {
        //conn_t http;

        memset(&http, 0, sizeof(conn_t));
        http.reqType = HTTP;
        http.host = (char *) 1; //如果为NULL，modify_request函数可能会使其指向动态分配内存
        if (modify_request(strdup(HTTPDNS_REQUEST), strlen(HTTPDNS_REQUEST), &http) != 0)
            return 1;
        this->dns.http_req = str_replace(http.ready_data, "[H]", dest);
        if (this->dns.http_req == NULL)
            return 1;
        if (this->mode == WAP || (this->mode == NET_PROXY && port != 80 && port != 8080))
            memcpy(&this->dns.dst, &this->http.dst, sizeof(this->dns.dst));
    } else {
        this->dns.http_req = str_replace(this->dns.http_req, "[M]", "GET");
        this->dns.http_req = str_replace(this->dns.http_req, "[url]", "/d?dn=[D]");
        this->dns.http_req = str_replace(this->dns.http_req, "[U]", "/d?dn=[D]");
        this->dns.http_req = str_replace(this->dns.http_req, "[V]", "HTTP/1.0");;
        this->dns.http_req = str_replace(this->dns.http_req, "[H]", dest);
    }
    if (this->mode == WAP_CONNECT || (this->mode == NET_CONNECT && port != 80 && port != 8080)) {
        //构建CONNECT请求头
        this->dns.connect_req = str_replace(strdup(def_ssl_req), "[H]", dest);
        if (this->dns.connect_req == NULL)
            return 1;
        this->dns.connect_req_len = strlen(this->dns.connect_req);
        memcpy(&this->dns.dst, &this->https.dst, sizeof(this->dns.dst));
    }
*/
    return 0;
}

char *conf::str_replace(char *str, const char *src,const char *dest) {
    if (!str || !dest)
        return str;

    static char *p;
    static int diff, src_len, dest_len;

    p = strstr(str, src);
    if (p == NULL)
        return str;
    src_len = strlen(src);
    dest_len = strlen(dest);

    if (src_len == dest_len) {
        for (; p; p = strstr(p, src)) {
            memcpy(p, dest, dest_len);
            p += dest_len;
        }
    } else if (src_len < dest_len) {
        static int str_len, before_len;
        static char *before_end, *new_str;

        diff = dest_len - src_len;
        for (str_len = strlen(str); p; p = strstr(p, src)) {
            str_len += diff;
            before_len = p - str;
            new_str = (char *) realloc(str, str_len + 1);
            if (new_str == NULL) {
                free(str);
                return NULL;
            }
            str = new_str;
            p = str + str_len;
            before_end = str + before_len;
            while (p - dest_len + 1 != before_end) {
                *p = *(p - diff);
                p--;
            }
            memcpy(before_end, dest, dest_len);
        }
    } else if (src_len > dest_len) {
        diff = src_len - dest_len;
        for (; p; p = strstr(p, src)) {
            memcpy(p, dest, dest_len);
            p += dest_len;
            strcpy(p, p + diff);
        }
    }

    return str;
}


