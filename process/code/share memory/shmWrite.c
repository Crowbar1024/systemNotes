#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "shmdata.h"
 
int main() {
	// 1. 取得共享内存
	int shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);
	if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	// 2. 将共享内存连接到当前进程的地址空间
	void *shm = shmat(shmid, NULL, 0);
	if (shm == (void*) -1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
    // 3. 写数据
	struct shared_use_st *shared = (struct shared_use_st*) shm;
    int running = 1;
    char buffer[BUFSIZ + 1];//用于保存输入的文本
	while (running) {
		while (shared->m_flag == canRead) {
			sleep(1);
			printf("Waiting...\n");
		}
		printf("Enter some text: ");
		fgets(buffer, BUFSIZ, stdin);
		strncpy(shared->text, buffer, TEXT_SZ);
		shared->m_flag = canRead;  // 写完数据，设置m_flag使共享内存段可读
		if (strncmp(buffer, "end", 3) == 0) {  // 输入了end，退出循环（程序）
            running = 0;
        }
	}

	// 4. 把共享内存从当前进程中分离
	if (shmdt(shm) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	sleep(2);
	exit(EXIT_SUCCESS);
}

/*
	ipcs -m
	可以发现有两个连接数。
*/