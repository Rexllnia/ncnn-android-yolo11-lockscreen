#include "api_server.h"

static int server_socket = -1;
static char *g_content= NULL;

void api_server_set_content(const char *content) {
    if (g_content) {
        free(g_content);
    }

    g_content = calloc(1, strlen(content));
    strcpy(g_content,content);

    return;
}

// 发送HTTP响应的辅助函数
void send_http_response(int client_socket, const char* content) {
    if  (content == NULL) {
        return;
    }
    char *response = calloc(1, 1024 + strlen(content));
    int length = sprintf(response,
                          "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain; charset=utf-8\r\n"
                          "Content-Length: %zu\r\n"
                          "Connection: close\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "\r\n"
                          "%s",
                          strlen(content), content);

    send(client_socket, response, length, 0);
    free(response);
}

// 处理客户端连接的函数
void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    free(arg); // 释放动态分配的内存

    char buffer[1024] = {0};

    // 读取HTTP请求（这里简单读取，实际项目可能需要更完整的解析）
    read(client_socket, buffer, sizeof(buffer) - 1);


    send_http_response(client_socket,g_content);

    // 关闭客户端套接字
    close(client_socket);
    return NULL;
}

// 服务器主循环
void* server_loop(void* arg) {
    int port = 1234;

    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    // 创建套接字
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        return (void*)-1;
    }

    // 设置套接字选项
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        close(server_socket);
        return (void*)-1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 绑定套接字
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(server_socket);
        return (void*)-1;
    }

    // 开始监听
    if (listen(server_socket, 3) < 0) {
        close(server_socket);
        return (void*)-1;
    }

    // 服务器主循环
    while (1) {
        int* client_socket = malloc(sizeof(int));
        if ((*client_socket = accept(server_socket, (struct sockaddr*)&address,
                                     (socklen_t*)&addrlen)) >= 0) {
            // 为每个客户端连接创建新线程处理
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handle_client, client_socket);
            pthread_detach(client_thread); // 分离线程，使其结束后自动释放资源
        } else {
            free(client_socket);
        }
    }

    close(server_socket);
    server_socket = -1;
    return (void*)0;
}



char* get_local_ips() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    static char ips[1024] = {0}; // 静态数组存储所有IP
    char temp[256];

    if (getifaddrs(&ifaddr) == -1) {
        return "无法获取网络接口信息";
    }

    strcpy(ips, "可用的IP地址:\n");

    // 遍历所有网络接口
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // 只处理IPv4和IPv6地址
        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) :
                            sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if (s == 0) {
                // 过滤掉回环地址和无效接口
                if (strcmp(ifa->ifa_name, "lo") != 0 &&
                    strncmp(ifa->ifa_name, "dummy", 5) != 0) {

                    // 获取地址类型描述
                    const char* family_str = (family == AF_INET) ? "IPv4" : "IPv6";

                    snprintf(temp, sizeof(temp), "接口 %s (%s): %s\n",
                             ifa->ifa_name, family_str, host);

                    // 检查缓冲区空间是否足够
                    if (strlen(ips) + strlen(temp) < sizeof(ips) - 1) {
                        strcat(ips, temp);
                    }
                }
            }
        }
    }

    freeifaddrs(ifaddr);

    // 如果没有找到有效的IP地址
    if (strlen(ips) <= strlen("可用的IP地址:\n")) {
        strcpy(ips, "未找到有效的网络接口或IP地址");
    }

    return ips;
}
