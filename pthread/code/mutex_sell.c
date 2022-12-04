#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/*
    3个人卖1000张票，每卖一张输出谁卖了第几张
*/

int tickets = 1000;  // 全局变量，所有的线程都共享这一份资源
pthread_mutex_t mutex;

void * sellticket(void* arg) {
    while(1) {
        pthread_mutex_lock(&mutex);
        if (tickets > 0) {
            usleep(6000);
            printf("%ld 正在卖第 %d 张门票\n", pthread_self(), tickets);
            tickets--;
        } else {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex, NULL);

    // 创建3个子线程
    pthread_t tid1, tid2, tid3;
    pthread_create(&tid1, NULL, sellticket, NULL);
    pthread_create(&tid2, NULL, sellticket, NULL);
    pthread_create(&tid3, NULL, sellticket, NULL);

    // 回收子线程的资源,阻塞
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);

    pthread_exit(NULL); // 退出主线程

    pthread_mutex_destroy(&mutex);
    return 0;
}

/*
    每次某个线程会一下子卖很多张票，才轮到其他线程。然后依此往复
*/
