//
// Created by zhengyufan on 2026/1/15.
//

#include "spsc.h"


void spsc_init(spsc_queue_t* q, void** buf, uint32_t size)
{
    // size 必须是 2 的幂
    q->size = size;
    q->mask = size - 1;
    atomic_store_explicit(&q->head, 0, memory_order_relaxed);
    atomic_store_explicit(&q->tail, 0, memory_order_relaxed);
    q->buffer = buf;
}

int spsc_push(spsc_queue_t* q, void* item)
{
    uint32_t head = atomic_load_explicit(&q->head, memory_order_relaxed);
    uint32_t next = (head + 1) & q->mask;

    uint32_t tail = atomic_load_explicit(&q->tail, memory_order_acquire);
    if (next == tail)
        return 0; // full

    q->buffer[head] = item;

    atomic_store_explicit(&q->head, next, memory_order_release);
    return 1;
}

int spsc_pop(spsc_queue_t* q, void** item)
{
    uint32_t tail = atomic_load_explicit(&q->tail, memory_order_relaxed);

    uint32_t head = atomic_load_explicit(&q->head, memory_order_acquire);
    if (tail == head)
        return 0; // empty

    *item = q->buffer[tail];

    atomic_store_explicit(&q->tail, (tail + 1) & q->mask, memory_order_release);
    return 1;
}
