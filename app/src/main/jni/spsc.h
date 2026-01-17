//
// Created by zhengyufan on 2026/1/15.
//

#ifndef NCNN_ANDROID_YOLOV8_NEW_SPSC_H
#define NCNN_ANDROID_YOLOV8_NEW_SPSC_H

#endif //NCNN_ANDROID_YOLOV8_NEW_SPSC_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t size;
    uint32_t mask;
    _Atomic uint32_t head;
    _Atomic uint32_t tail;
    void** buffer;
} spsc_queue_t;

void spsc_init(spsc_queue_t* q, void** buf, uint32_t size);
int spsc_push(spsc_queue_t* q, void* item);
int spsc_pop(spsc_queue_t* q, void** item);