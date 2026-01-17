#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*WatchdogCallback)(void* user);

typedef struct Watchdog Watchdog;

/**
 * 获取全局 Watchdog 实例
 * 参数：
 *   timeout_ms: 超时时间，单位毫秒
 *   cb: 超时回调函数
 *   user: 回调函数用户数据
 */
Watchdog* watchdog_get_instance(uint32_t timeout_ms, WatchdogCallback cb, void* user);

/**
 * 喂狗（重置计时器）
 */
void watchdog_kick(Watchdog* wd);

#ifdef __cplusplus
}
#endif

#endif // WATCHDOG_H
