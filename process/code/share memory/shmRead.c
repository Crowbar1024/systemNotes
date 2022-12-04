#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "shmdata.h"

int main() {
	// 1. 创建共享内存
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

	// 3. 读数据
	struct shared_use_st *shared = (struct shared_use_st*) shm;
	shared->m_flag = canWrite;
    int running = 1;
	while (running) {
		if (shared->m_flag == canRead) {  // 没有进程写数据，所以可以读数据
			printf("You wrote: %s", shared->text);
			sleep(rand() % 3);
			shared->m_flag = canWrite;  // 读完了，其他人可以写了
			if (strncmp(shared->text, "end", 3) == 0) {  // 输入了end，退出循环（程序）
				running = 0;
            }
		} else {  //有其他进程在写数据，不能读取数据
			sleep(1);
        }
	}

	// 4. 把共享内存从当前进程中分离
	if (shmdt(shm) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	// 5. 删除共享内存
	if (shmctl(shmid, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}