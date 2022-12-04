#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// 使用内存映射实现文件拷贝
int main() {

    // 1. 对原始的文件进行内存映射
    int fd = open("english.txt", O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(0);
    }
    int len = lseek(fd, 0, SEEK_END);

    // 2. 创建一个新文件
    int fd1 = open("cpy.txt", O_RDWR | O_CREAT, 0664);
    if (fd1 == -1) {
        perror("open");
        exit(0);
    }
    truncate("cpy.txt", len);  // 对新创建的文件进行拓展
    write(fd1, " ", 1);

    // 3. 分别做内存映射
    void* ptr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(0);
    }
    void* ptr1 = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    if (ptr1 == MAP_FAILED) {
        perror("mmap");
        exit(0);
    }

    // 4. 将第一个文件的内存数据拷贝到新的文件内存中
    memcpy(ptr1, ptr, len);
    
    // 5. 释放资源
    munmap(ptr1, len);
    munmap(ptr, len);
    close(fd1);
    close(fd);

    return 0;
}