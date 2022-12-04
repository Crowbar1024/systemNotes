#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// 模拟 ls-lR | wc -l
// 即递归计数所有普通文件的数量
// wc是计数，-l表示行数。
int getFileNum(const char * path);

// 读取某个目录下所有的普通文件的个数
int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("%s path\n", argv[0]);
        return -1;
    }
    int num = getFileNum(argv[1]);
    printf("普通文件的个数为：%d\n", num);
    return 0;
}

// 用于获取目录下所有普通文件的个数
int getFileNum(const char * path) {
    DIR * dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        exit(0);
    }
    struct dirent *ptr;
    int total = 0;  // 记录普通文件的个数
    while((ptr = readdir(dir)) != NULL) {
        char * dname = ptr->d_name;  // 获取名称

        if(strcmp(dname, ".") == 0 || strcmp(dname, "..") == 0) {  // 忽略上级目录和本级
            continue;
        } else if(ptr->d_type == DT_DIR) {  // 目录，需要递归
            char newpath[256];
            sprintf(newpath, "%s/%s", path, dname);
            total += getFileNum(newpath);
        } else if(ptr->d_type == DT_REG) {  // 普通文件
            total++;
        }
    }
    // 关闭目录
    closedir(dir);
    return total;
}