//
// Created by zhengyufan on 2026/1/6.
//

#ifndef NCNN_ANDROID_YOLOV8_NEW_TCP_CLIENT_H
#define NCNN_ANDROID_YOLOV8_NEW_TCP_CLIENT_H

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
#include <usock.h>
#include <sys/socket.h>
#include <android/log.h>
#include <stdbool.h>
#include "spsc.h"

void tcp_client_init ();
void tcp_client_add_element (const char *s);

#endif //NCNN_ANDROID_YOLOV8_NEW_TCP_CLIENT_H
