#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

/*
    3个生产者各自生产3个数，当生产完后，用一个消费者把它吃掉。
    用一个信号量去调度线程。初始化为0，每次生产玩+1。那么消费者只需要执行3次-1操作就行。
*/

sem_t psem;
int nums[3];

void* producer(void* arg) {
    unsigned long pid = (unsigned long) arg;
    int idx = (int) pid;
    nums[idx] = rand()%100;
    sem_post(&psem);
    printf("producer %d produced item %d.\n", idx, nums[idx]);
    return NULL;
}

void* customer(void* arg) {
    for (int i = 0; i < 3; ++i) {
        sem_wait(&psem);
    }
    int res = 0;
    for (int i = 0; i < 3; ++i) {
        res += nums[i];
    }
    printf("costomer eat %d totally.\n", res);
    return NULL;
}


int main() {
    sem_init(&psem, 0, 0);  // 初始值必须为0

    pthread_t ptid[3], ctid;
    for (int i = 0; i < 3; i++) {
        void* arg = (void*)(unsigned long) i;
        pthread_create(&ptid[i], NULL, producer, arg);
    }
    pthread_create(&ctid, NULL, customer, NULL);
    
    for(int i = 0; i < 3; i++) {
        pthread_join(ptid[i], NULL);
    }
    pthread_join(ctid, NULL);

    return 0;
}

/*
    producer 0 produced item 83.
    producer 2 produced item 86.
    producer 1 produced item 77.
    costomer eat 246 totally.
*/