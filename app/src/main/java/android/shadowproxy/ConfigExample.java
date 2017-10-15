package android.shadowproxy;

public class ConfigExample {
    public static String example=
            "//分为4个模块：global、http、https、udp_ontcp\n" +
                    "//http和https模块的set_first del_hdr strrep regrep语法从上往下执行\n" +
                    "//只有双引号引住的变量才支持换行\n" +
                    "\n" +
                    "//全局模块\n" +
                    "global {\n" +
                    "    //设置运行uid，不支持\n" +
                    "    uid = 3004;\n" +
                    "    \n" +
                    "    //http处理模式[wap wap_connect net_proxy net_connect] 不设置则为net\n" +
                    "    //wap: 所有请求走代理ip\n" +
                    "    //wap_connect: 所有请求走https代理ip\n" +
                    /*"    //net_proxy: HTTP请求80 8080端口直连目标服务器，其他端口走http代理ip\n" +
                    "    //net_connect : HTTP请求80 8080端口直连目标服务器，其他端口走https代理ip\n" +*/
                    "    //net: HTTP请求直连目标服务器\n" +
                    "    //raw: 所有请求直连目标服务器\n" +
                    "    mode = wap;\n" +
                    "\n" +
                    "    //TCP，DNS监听地址，不填IP则为默认IP，不支持\n" +
                    "    tcp_listen = 10086;\n" +
                    "    dns_listen = 10086;\n" +
                    "\n" +
                    "    //进程数，不支持\n" +
                    "    procs = 2;\n" +
                    "    \n" +
                    "    //严格修改请求头，对于一次读取数据连续的多个请求头（比如qq浏览器加载\"看热点\"），全部修改，默认只修改第一个请求头\n" +
                    "    strict = on;\n" +
                    "}\n" +
                    "\n" +
                    "//http模块\n" +
                    "http {\n" +
                    "    //普通http请求只留GET POST联网\n" +
                    "    only_get_post = on;\n" +
                    "    \n" +
                    "    //http端口，其他端口先建立CONNECT连接\n" +
                    "    http_port = 80,8080,10086;\n" +
                    "    \n" +
                    "    // http目标地址\n" +
                    "    addr = 10.0.0.172:80;\n" +
                    "    \n" +
                    "    //删除Host行，不区分大小写\n" +
                    "    del_hdr = host;\n" +
                    "    del_hdr = X-Online-Host;\n" +
                    "    \n" +
                    "    //如果搜索到以下字符串则进行https代理(net模式下无效)\n" +
                    "    proxy_https_string = WebSocket;\n" +
                    "    proxy_https_string = Upgrade:;\n" +
                    "    \n" +
                    "    以下语法特有: [M]: method，[H]: host，[U]: uri，[url]: url，[V]: protocol\n" +
                    "    //设置首行\n" +
                    "    set_first = \"[M] [U] [V]\\r\\n Host: rd.go.10086.cn\\r\\n\";\n" +
                    "    //字符串替换，区分大小写\n" +
                    "    //strrep = \"Host:\" -> \"Cloud:\";\n" +
                    "    \n" +
                    "    //正则表达式替换，不区分大小写\n" +
                    "    //regrep = \"^Host:[^\\n]*\\n\" -> \"Meng: [H]\\r\\n\";\n" +
                    "}\n" +
                    "\n" +
                    "//https模块，没有only_get_post、http_port和download_max_size，其他语法跟http一样\n" +
                    "[区别] 注意: !!!https模块为原始CONNECT请求。\n" +
                    "https {\n" +
                    "    addr = 10.0.0.172:80;\n" +
                    "    del_hdr = host;\n" +
                    "    set_first = \"CONNECT /rd.go.10086.cn HTTP/1.1\\r\\nHost: [H]\\r\\nProxy-Connection: Keep-Alive\\r\\n\\r\\n\";\n" +
                    "}\n" +
                    "\n" +
                    "//httpDNS模块，暂不支持。\n" +
                    "udp_ontcp {\n" +
                    "    //http请求目标地址\n" +
                    "    addr = 182.254.118.118;\n" +
                    "    //缓存路径，关闭的时候不要加-9，否则缓存无法写入缓存\n" +
                    "    //cachePath = dns.cache;\n" +
                    "    //限制缓存数目\n" +
                    "    //cacheLimit = 64;\n" +
                    "    //http请求头，不设置则用http模块修改后的默认请求，[D]为查询的域名\n" +
                    "    //http_req = \"[M] http://rd.go.10086.cn/d?dn=[D] [V]\\r\\nHost: rd.go.10086.cn\\r\\nConnection: close\\r\\n\\r\\n\";\n" +
                    "}";
}
