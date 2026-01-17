#include "watchdog.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <threads.h>
#include <time.h>
#include <stdint.h>

struct Watchdog {
    uint32_t timeout_ms;
    WatchdogCallback callback;
    void* user;

    atomic_int running;
    atomic_uint_least64_t last_kick_ms;

    thrd_t thread;
};

static Watchdog* g_watchdog = NULL;

// 获取当前时间毫秒
static uint64_t now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Watchdog 循环线程
static int watchdog_thread_func(void* arg) {
    Watchdog* wd = (Watchdog*)arg;

    while (atomic_load(&wd->running)) {
        uint64_t last = atomic_load(&wd->last_kick_ms);
        uint64_t curr = now_ms();

        if ((curr - last) > wd->timeout_ms) {
            if (wd->callback)
                wd->callback(wd->user);

            // 喂自己一次，避免重复触发
            atomic_store(&wd->last_kick_ms, curr);
        }

        thrd_sleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 50 * 1000000 }, NULL); // 50ms
    }

    return 0;
}

Watchdog* watchdog_get_instance(uint32_t timeout_ms, WatchdogCallback cb, void* user) {
    if (g_watchdog)
        return g_watchdog;

    g_watchdog = (Watchdog*)malloc(sizeof(Watchdog));
    if (!g_watchdog)
        return NULL;

    g_watchdog->timeout_ms = timeout_ms;
    g_watchdog->callback = cb;
    g_watchdog->user = user;
    atomic_store(&g_watchdog->running, 1);
    atomic_store(&g_watchdog->last_kick_ms, now_ms());

    if (thrd_create(&g_watchdog->thread, watchdog_thread_func, g_watchdog) != thrd_success) {
        free(g_watchdog);
        g_watchdog = NULL;
        return NULL;
    }

    return g_watchdog;
}

void watchdog_kick(Watchdog* wd) {
    if (!wd)
        wd = g_watchdog;

    if (!wd)
        return;

    atomic_store(&wd->last_kick_ms, now_ms());
}
