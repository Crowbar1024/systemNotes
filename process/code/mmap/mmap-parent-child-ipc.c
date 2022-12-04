#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

// 建立父子进程之间的内存映射，子进程直接在映射区修改内容
int main() {
    int fd = open("test.txt", O_RDWR);
    int size = lseek(fd, 0, SEEK_END);  // 获取文件的大小

    // 创建内存映射区
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(0);
    }

    pid_t pid = fork();
    if (pid > 0) {
        wait(NULL);
        char buf[64];
        strcpy(buf, (char *)ptr);
        printf("read data : %s\n", buf);
    } else if(pid == 0){
        strcpy((char *)ptr, "yhnb");
    }

    // 关闭内存映射区
    munmap(ptr, size);
    return 0;
}