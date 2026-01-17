//
// Created by zhengyufan on 2026/1/6.
//

#include <semaphore.h>

#include "tcp_client.h"
static pthread_t client_thread;
static int client_fd;
static sem_t sem;
void* ringbuf[8];   // 8 帧缓存
spsc_queue_t q;

void * client_loop () {
//
    char *out;

    while(1) {
        sem_wait(&sem); /* 等待链表不为空 */
        spsc_pop(&q, (void**)&out);
retry:
        if (send(client_fd, out, strlen(out), 0) < 0) {
            close(client_fd);
            sleep(2);
            client_fd = usock(USOCK_TCP | USOCK_IPV6ONLY | USOCK_NUMERIC, "240e:379:1b2:2200::100", "9000");
            goto retry;
        } else {
            free(out);
        }
    }


    return NULL;
}


void tcp_client_add_element (const char *s) {
    char* msg = malloc(strlen(s)+1);
    strcpy(msg, s);
    spsc_push(&q, msg);
}



void tcp_client_init () {
    char* msg = malloc(16);
    char *out;
    sem_init(&sem, 0, 0);
    spsc_init(&q, ringbuf, 8);

    client_fd = usock(USOCK_TCP | USOCK_IPV6ONLY | USOCK_NUMERIC, "240e:379:1b2:2200::100", "9000");
    pthread_create(&client_thread, NULL, client_loop, NULL);
}