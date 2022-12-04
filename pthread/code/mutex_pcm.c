#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/*
    Producer-consumer model
    1个生产者，5个消费者
    只使用互斥锁

    原理是一个链表，每次生产新的节点，放到头节点之前，再更新头节点
    消费就释放头节点
*/

pthread_mutex_t mutex;
int producedTotalNum = 10;
int producedCnt = 0;

struct Node{
    int num;
    struct Node *next;
};
struct Node* head = NULL;

// 不断的创建新的节点，添加到链表的头部
void* producer(void* arg) {
    while (++producedCnt <= producedTotalNum) {
        pthread_mutex_lock(&mutex);
        struct Node* cur = (struct Node*)malloc(sizeof(struct Node));
        cur->num = producedCnt;
        cur->next = head;
        head = cur;
        printf("producer produced item %d\n", cur->num);
        pthread_mutex_unlock(&mutex);
        sleep(0.2);  // 1s，放慢生产和消费的速度
    }
    printf("producer can't produce any more item.\n");
    return NULL;
}

void* customer(void* arg) {
    unsigned long cid = (unsigned long) arg;
    printf("costomer %ld is ready.\n", cid+1);
    while (producedCnt > 0) {
        pthread_mutex_lock(&mutex);
        struct Node* tmp = head;  // 保存头结点，用于删除
        if (head != NULL) {  // 有数据，可以消费
            head = head->next;
            printf("costomer %ld consumed item %d\n", cid+1, tmp->num);
            free(tmp);
            pthread_mutex_unlock(&mutex);
        } else {
            pthread_mutex_unlock(&mutex);
        }
    }
    return  NULL;
}

int main() {
    printf("There are %d items to produce and comsume.\n", producedTotalNum);

    pthread_t ptid;
    pthread_create(&ptid, NULL, producer, NULL);
    pthread_t ctids[5];
    for (int i = 0; i < 5; i++) {
        void* arg = (void*)(unsigned long) i;
        pthread_create(&ctids[i], NULL, customer, arg);
    }

    // pthread_detach(ptid);
    // for(int i = 0; i < 5; i++) {
    //     pthread_detach(ctids[i]);
    // }

    pthread_detach(ptid);
    for(int i = 0; i < 5; i++) {
        pthread_join(ctids[i], NULL);
    }

    pthread_exit(NULL);
    return 0;
}

/*  
    输出
    There are 10 items to produce and comsume.
    costomer 2 is ready.
    producer produced item 1
    costomer 3 is ready.
    costomer 1 is ready.
    costomer 5 is ready.
    costomer 4 is ready.
    costomer 2 consumed item 1
    producer produced item 2
    costomer 3 consumed item 2
    producer produced item 3
    costomer 4 consumed item 3
    producer produced item 4
    costomer 3 consumed item 4
    producer produced item 5
    costomer 3 consumed item 5
    producer produced item 6
    costomer 3 consumed item 6
    producer produced item 7
    costomer 3 consumed item 7
    producer produced item 8
    costomer 3 consumed item 8
    producer produced item 9
    costomer 3 consumed item 9
    producer produced item 10
    costomer 3 consumed item 10
    producer can't produce any more item.

    可以发现：线程3很猛，抢到了几乎所有商品

    总结：只有互斥锁，很多场景就会耗用很多无用功。这里生产者每0.2s生产一个商品，消费者如果没有延迟，就会不断轮询去问有没有商品，
    每次都要加解锁，消耗非常多的资源。
*/