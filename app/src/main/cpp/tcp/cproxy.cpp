//
// Created by lovefantasy on 2017/6/2.
//

#include "cproxy.h"
#include "../utils/nat_mgr.h"

cproxy::cproxy(int tcp_fd, int fd) : tcp_base(tcp_fd, fd) {

}

cproxy::cproxy(int tcp_fd, sockaddr_in *dst, tcpsession *s)
        : tcp_base(tcp_fd, dst) {
    this->s = s;

}

void cproxy::connect_remote(){
    this->req_type = s->request_type;

    /**
    wap: 所有请求走代理ip
    wap_connect: 所有请求走https代理ip
    net_proxy: HTTP请求80 8080端口直连目标服务器，其他端口走http代理ip
    net_connect : HTTP请求80 8080端口直连目标服务器，其他端口走https代理ip
    net: HTTP请求直连目标服务器
     **/
    switch (c->mode) {
        case WAP:
            if (req_type == HTTP || req_type == HTTP_OTHERS) {
                m_proxy_addr = &c->http.dst;
            } else if (req_type == SSL || req_type == HTTP_CONNECT) {
                m_proxy_addr = &c->https.dst;
            }
            break;
        case NET:
            if (req_type == HTTP || req_type == HTTP_OTHERS) {
                m_proxy_addr = m_remote_addr;
            } else if (req_type == SSL || req_type == HTTP_CONNECT) {
                m_proxy_addr = &c->https.dst;
            }
            break;
        case NET_PROXY:
            if (m_remote_addr->sin_port == htons(80) || m_remote_addr->sin_port == htons(8080)) {
                m_proxy_addr = m_remote_addr;
            } else {
                m_proxy_addr = &c->http.dst;
            }
            break;
        case NET_CONNECT:
            if (m_remote_addr->sin_port == htons(80) || m_remote_addr->sin_port == htons(8080)) {
                m_proxy_addr = m_remote_addr;
            } else {
                m_proxy_addr = &c->https.dst;
            }
            break;
        case WAP_CONNECT:
            m_proxy_addr = &c->https.dst;
            break;
        case RAW:
            m_proxy_addr = m_remote_addr;
            break;
        default:
            m_proxy_addr = m_remote_addr;
            break;
    }
}

/*** 连接服务器成功***/
void cproxy::on_connect() {
    switch (c->mode) {
        case WAP:
            if (req_type == HTTP || req_type == HTTP_OTHERS || req_type == HTTP_CONNECT) {
                m_begin_write_pair = true;
                begin_contact();
            } else if (req_type == SSL) {
                send_connect();
            }
            break;
        case NET:
            if (req_type == HTTP || req_type == HTTP_OTHERS || req_type == HTTP_CONNECT) {
                m_begin_write_pair = true;
                begin_contact();
            } else if (req_type == SSL) {
                send_connect();
            }
            break;
        case NET_PROXY:
            if (m_remote_addr->sin_port == htons(80) || m_remote_addr->sin_port == htons(8080)) {
                m_begin_write_pair = true;
                begin_contact();
            } else {
                if (req_type == HTTP || req_type == HTTP_OTHERS || req_type == HTTP_CONNECT) {
                    m_begin_write_pair = true;
                    begin_contact();
                } else if (req_type == SSL) {
                    send_connect();
                }
            }
            break;
        case NET_CONNECT:
            if (m_remote_addr->sin_port == htons(80) || m_remote_addr->sin_port == htons(8080)) {
                m_begin_write_pair = true;
                begin_contact();
            } else {
                if (req_type == HTTP || req_type == HTTP_OTHERS || req_type == HTTP_CONNECT) {
                    m_begin_write_pair = true;
                    begin_contact();
                } else if (req_type == SSL) {
                    send_connect();
                }
            }
            break;
        case WAP_CONNECT:
            send_connect();
            break;
        case RAW:
            m_begin_write_pair = true;
            begin_contact();
            break;
        default:
            m_begin_write_pair = true;
            begin_contact();
            break;
    }
}


void cproxy::send_connect() {
    regist_read();
    modify *mod = c->https.m;
    while (mod->flag != SET_FIRST) {
        mod = mod->next;
    }
    m_ready_len = strlen(mod->str) + 3;
    m_ready_data = (char *) malloc((size_t)m_ready_len);
    strcpy(m_ready_data, mod->str);
    char *tmp = m_ready_data + m_ready_len;
    *(tmp - 1) = '\0';
    *(tmp - 2) = '\n';
    *(tmp - 3) = '\r';
    m_ready_data = str_replace(m_ready_data, "[H]",
                              splice_ip_port(inet_ntoa(m_remote_addr->sin_addr),
                                             ntohs(m_remote_addr->sin_port)));
    m_ready_len = strlen(m_ready_data);
    write_data(m_ready_data,(size_t)m_ready_len);
}

/*** 发送请求之前***/
int cproxy::encrypt(const char* data,size_t data_len) {
    if (req_type == SSL || c->mode == RAW) {
        m_ready_data = (char*)data;
        m_ready_len = data_len;
        return 0;
    }
    if (!modify_request(data,data_len)) {
        return 0;
    } else {
        return 1;
    }

}

/*** 收到回应之后***/
int cproxy::decrypt() {
    if (m_is_first) {
        m_is_first = false;
        switch (c->mode) {
            case WAP:
                if (req_type == SSL || req_type == HTTP_CONNECT) {
                    return is_connectec(m_recv_data);
                }
                break;
            case NET:
                if (req_type == SSL || req_type == HTTP_CONNECT) {
                    return is_connectec(m_recv_data);
                }
                break;
            case NET_PROXY:
                if (m_remote_addr->sin_port != htons(80) &&
                    m_remote_addr->sin_port != htons(8080)) {
                    if (req_type == SSL || req_type == HTTP_CONNECT) {
                        return is_connectec(m_recv_data);
                    }
                }
                break;
            case NET_CONNECT:
                if (m_remote_addr->sin_port != htons(80) &&
                    m_remote_addr->sin_port != htons(8080)) {
                    if (req_type == SSL || req_type == HTTP_CONNECT) {
                        return is_connectec(m_recv_data);
                    }
                }
                break;
            case WAP_CONNECT:
                return is_connectec(m_recv_data);
            case RAW:
                return 0;
            default:
                return 0;
        }
    } else if (!m_begin_write_pair) {
        m_begin_write_pair = true;
    }
    return 0;
}

/*** 连接关闭***/
//

int8_t cproxy::modify_request(const char* data,size_t data_len) {
    static struct modify *mod;
    static char *tmp_m, *new_header;
    static size_t string_len;
    static int req_type_per = 0;
    this->m_ready_data = 0;
    req_type_per = get_type(data);
    switch (req_type_per) {
        case HTTP_OTHERS:
            if (c->http_only_get_post) {
                m_ready_data = (char*)data;
                m_ready_len = data_len;
                return 0;
            }
        case HTTP:
            mod = c->http.m;
            if (parse_http_request(data, data_len)) {
                goto error;
            }
            break;
        case HTTP_CONNECT:
            mod = c->https.m;
            if (parse_connect_request(data, data_len) != 0) {
                goto error;
            }
            break;
        default:
            m_ready_data = (char*)data;
            m_ready_len = data_len;
            return 0;
    }

    while (mod) {
        switch (mod->flag) {
            case DEL_HDR:
                if (del_hdr(header, mod) != 0) {
                    goto error;
                }
                while (mod->next && mod->next->flag == DEL_HDR)
                    mod = mod->next;
                break;

            case SET_FIRST:
                tmp_m = strchr(header, '\n') + 1;
                string_len = strlen(mod->str);
                new_header = (char *) malloc(string_len + strlen(tmp_m) + 1);
                if (new_header == NULL) {
                    LOGE("cproxy", " modify %s", strerror(errno));
                    goto error;
                }
                strcpy(new_header, mod->str);
                strcpy(new_header + string_len, tmp_m);
                /***旧的header没释放*/
                free(header);
                /******************/
                header = new_header;
                break;

            case STRREP:
                header = str_replace(header, mod->str, mod->dest);
                if (header == NULL) {
                    goto error;
                }
                break;
            default:
                header = reg_strrep(header, &mod->reg_src, mod->dest);
                if (header == NULL)
                    goto error;
                break;
        }
        mod = mod->next;
    }
    if (req_type_per != HTTP_CONNECT) {
        header = str_replace(header, "[M]", method);

        header = str_replace(header, "[U]", uri);

        header = str_replace(header, "[url]", url);

        header = str_replace(header, "[V]", version);
    }
    header = str_replace(header, "[H]", host);

    header_len = strlen(header);

    if (other) {
        tmp_m = (char *) realloc(header, header_len + other_len);
        if (tmp_m == NULL)
            goto error;
        header = tmp_m;
        memcpy(tmp_m + header_len, other, other_len);
        header_len += other_len;
    }
    /***将数据转移给ready_data***/
    this->m_ready_data = header;
    this->m_ready_len = header_len;

    /***手动重置，避免被本函数释放***/
    header = 0;
    /******/

    /***释放本轮资源***/
    free_http_request();
    return 0;

    error:
    free_http_request();
    return -1;
}

int8_t cproxy::parse_http_request(const char *request, size_t request_len) {
    static char *p;
    /* 分离请求头和请求数据 */
    if ((p = strstr((char *) request, "\n\r")) != NULL &&
        (header_len = p + 3 - request) < request_len) {
        if ((header = (char *) malloc(header_len + 1)) != NULL) {
            memcpy(header, request, header_len);
            *(header + header_len) = '\0';
        } else {
            return 1;
        }
        other_len = request_len - header_len;
        if ((other = (char *) malloc(other_len + 1)))
            memcpy(other, p + 3, other_len + 1);
        else
            return 1;
    } else {
        header_len = request_len;
        if ((header = (char *) malloc(header_len + 1))) {
            memcpy(header, request, header_len);
            *(header + header_len) = '\0';
        } else {
            return 1;
        }
    }

    /*获取method url version*/
    p = strchr(header, ' ');
    if (p) {
        method = strndup(header, p - header);
        version = strchr(++p, '\r'); //http版本后的\r
        if (version) {
            url = strndup(p, version - p - 9);
            version = strndup(version - 8, 8);
        }
    }

    host = get_host(header);
    //如果请求头中没有Host，则设置为原始IP和端口
    if (host == NULL)
        host = strdup(
                splice_ip_port(inet_ntoa(this->m_remote_addr->sin_addr),
                               this->m_remote_addr->sin_port));

    if (url) {
        if (*url != '/' && (p = strstr(url, "//")) != NULL) {
            p = strchr(p + 2, '/');
            if (p)
                uri = strdup(p);
            else
                uri = strdup("/");
        } else
            uri = url;
    }

    return 0;
}

int8_t cproxy::parse_connect_request(const char *request, size_t request_len) {
    static char *url_end; //pb0指向请求方法后的空格，pb1指向http版本后的空格

    url_end = strchr((char *) request + 8, ' ');
    if (url_end == NULL)
        return 1;
    this->host = strndup((char *) request + 8, url_end - (request + 8));
    if (this->host == NULL)
        return 1;

    if ((this->header = (char *) malloc(request_len + 1)) != NULL) {
        memcpy(this->header, request, request_len);
        *(header + request_len) = '\0';
    }
    this->header_len = request_len;

    return 0;
}

char *cproxy::reg_strrep(char *str, regex_t *src, char *dest) {
    if (!str)
        return NULL;

    static regmatch_t pm[10];
    static size_t before_len, dest_len, i;
    static ssize_t match_len;
    static char child_num[3] = {'\\', '0', '\0'}, *p, *real_dest, *match_end, save_match_end;
    static char *tmp_reg;
    p = str;
    while (regexec(src, p, 10, pm, 0) == 0) {
        //不进行不必要的字符串操作
        if (pm[1].rm_so >= 0) {
            /* 替换目标字符串中的子表达式 */
            real_dest = strdup(dest);
            if (real_dest == NULL) {
                free(str);
                return NULL;
            }
            for (i = 1; i < 10 && pm[i].rm_so >= 0; i++) {
                match_end = p + pm[i].rm_eo;
                save_match_end = *match_end;
                *match_end = '\0';
                child_num[1] = (char) (i + 48);
                tmp_reg = str_replace(real_dest, child_num, p + pm[i].rm_so);
                *match_end = save_match_end;
                if (tmp_reg == NULL) {
                    free(str);
                    return NULL;
                }
                real_dest = tmp_reg;
            }
        } else {
            real_dest = dest;
        }
        dest_len = strlen(real_dest);
        //regoff_t
        match_len = pm[0].rm_eo - pm[0].rm_so;
        p += pm[0].rm_so;
        //目标字符串不大于匹配字符串则不用分配新内存
        if (match_len >= dest_len) {
            memcpy(p, real_dest, dest_len);
            if (match_len > dest_len)
                strcpy(p + dest_len, p + match_len);
            p += dest_len;
        } else {
            static size_t str_len, diff;
            static char *before_end, *new_str;

            diff = dest_len - match_len;
            str_len = strlen(str) + diff;
            before_len = p - str;
            new_str = (char *) realloc(str, str_len + 1);
            if (new_str == NULL) {
                free(str);
                if (pm[1].rm_so >= 0)
                    free(real_dest);
                return NULL;
            }
            str = new_str;
            before_end = str + before_len;
            p = str + str_len;
            while (p - dest_len + 1 != before_end) {
                *p = *(p - diff);
                p--;
            }
            memcpy(before_end, real_dest, dest_len);
        }
        if (pm[1].rm_so >= 0)
            free(real_dest);
    }

    return str;
}

char *cproxy::splice_ip_port(char *ip, u_int16_t port) {
    static char original_ip_port[22];
    static char *p;
    static int a_unit;
    size_t ip_len;

    strcpy(original_ip_port, ip);
    ip_len = strlen(ip);
    original_ip_port[ip_len] = ':';
    p = original_ip_port + ip_len + 6;
    *p = '\0';
    for (a_unit = port % 10; port > 0; port /= 10, a_unit = port % 10)
        *(--p) = (char) (a_unit + 48);
    strcpy(original_ip_port + ip_len + 1, p);

    return original_ip_port;
}

char *cproxy::get_host(char *header) {
    static char *key, *host, *key_end, *host_end;
    static u_int key_len;

    host = NULL;
    for (key = strchr(header, '\n'); key++; key = strchr(key_end, '\n')) {
        key_end = strchr(key, ':');
        if (key_end == NULL)
            break;
        //部分请求头一行开头为:
        key_len = (uint) (key_end - key);
        if (key_len <= 0)
            continue;
        if (strncasecmp(key, "host", key_len) == 0) {
            host = key_end + 1;
            break;
        } else if (strncasecmp(key, "x-online-host", key_len) == 0) {
            host = key_end + 1;
            break;
        }
    }
    if (host == NULL)
        return NULL;
    while (*host == ' ')
        host++;
    host_end = strchr(host, '\r');
    if (host_end)
        return strndup(host, host_end - host);
    else
        return strdup(host);
}

int8_t cproxy::del_hdr(char *header, struct modify *head) {
    static struct modify *m;
    static char *key_end, *line_begin, *line_end;
    static uint key_len;

    for (line_begin = strchr(header, '\n'); line_begin; line_begin = line_end) {
        key_end = strchr(line_begin, ':');
        if (key_end == NULL)
            return 0;
        key_len = (u_int) (key_end - ++line_begin);
        line_end = strchr(key_end, '\n');
        m = head;
        do {
            if (strncasecmp(line_begin, m->str, key_len) == 0) {
                if (line_end) {
                    strcpy(line_begin, line_end + 1);
                    //新行前一个字符
                    line_end = line_begin - 1;
                } else
                    *line_begin = '\0';
                break;
            }
        } while ((m = m->next) != NULL && m->flag == DEL_HDR);
    }

    return 0;
}


char *cproxy::str_replace(char *str, const char *src, char *dest) {
    if (!str || !dest)
        return str;

    static char *p;
    static size_t diff, src_len, dest_len;

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
        static size_t str_len, before_len;
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


void cproxy::free_http_request() {
    if (header) {
        free(header);
        header = 0;
    }
    if (other) {
        free(other);
        other = 0;
    }
    if (method) {
        free(method);
        method = 0;
    }
    if (version) {
        free(version);
        version = 0;
    }
    if (uri && url != uri) {
        free(uri);
        uri = 0;
    }
    if (host) {
        free(host);
        host = 0;
    }
    if (url) {
        free(url);
        url = 0;
    }
}

u_int8_t cproxy::get_type(const char *req) {
    if (strncmp(req, "GET", 3) == 0 || strncmp(req, "POST", 4) == 0)
        return HTTP;
    else if (strncmp(req, "CONNECT", 7) == 0)
        return HTTP_CONNECT;
    else if (strncmp(req, "HEAD", 4) == 0 ||
             strncmp(req, "PUT", 3) == 0 ||
             strncmp(req, "OPTIONS", 7) == 0 ||
             strncmp(req, "MOVE", 4) == 0 ||
             strncmp(req, "COPY", 4) == 0 ||
             strncmp(req, "TRACE", 5) == 0 ||
             strncmp(req, "DELETE", 6) == 0 ||
             strncmp(req, "LINK", 4) == 0 ||
             strncmp(req, "UNLINK", 6) == 0 ||
             strncmp(req, "PATCH", 5) == 0 ||
             strncmp(req, "WRAPPED", 7) == 0)
        return HTTP_OTHERS;
    else
        return OTHER;
}

cproxy::~cproxy() {
    d_session = this->s;
}

bool cproxy::determine_connect(void *pVoid) {
    return c->mode == NET_PROXY || c->mode == NET_CONNECT || c->mode == RAW;
}

int cproxy::is_connectec(const char *data) {
    if (strstr(data, "HTTP/1.1 200")) {
        begin_contact();
        return 0;
    } else {
        return 1;
    }
    /*begin_contact();
    return 0;
*/
}









