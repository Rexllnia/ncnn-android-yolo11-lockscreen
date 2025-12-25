//
// Created by zhengyufan on 2025/12/5.
//

#ifndef NCNN_ANDROID_YOLOV8_API_SERVER_H
#define NCNN_ANDROID_YOLOV8_API_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>

void api_server_set_content(const char *text);
void* server_loop(void* arg);
char* get_local_ips();

#endif //NCNN_ANDROID_YOLOV8_API_SERVER_H
