#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t cond;

int producedTotalNum = 10;
int producedCnt = 0;

struct Node{
    int num;
    struct Node *next;
};
struct Node* head = NULL;

void* producer(void* arg) {
    while (++producedCnt <= producedTotalNum) {
        pthread_mutex_lock(&mutex);
        struct Node* cur = (struct Node*)malloc(sizeof(struct Node));
        cur->num = producedCnt;
        cur->next = head;
        head = cur;
        printf("producer produced item %d\n", cur->num);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);  // 只要生产了一个，就通知消费者消费
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
            sleep(0.2);
        } else {
            // 没有数据，需要等待
            pthread_cond_wait(&cond, &mutex);
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

    pthread_detach(ptid);
    for(int i = 0; i < 5; i++) {
        pthread_join(ctids[i], NULL);
    }

    return 0;
}

/*
    There are 10 items to produce and comsume.
    costomer 2 is ready.
    producer produced item 1
    costomer 2 consumed item 1
    costomer 3 is ready.
    costomer 4 is ready.
    costomer 5 is ready.
    costomer 1 is ready.
    producer produced item 2
    costomer 3 consumed item 2
    producer produced item 3
    costomer 3 consumed item 3
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


*/