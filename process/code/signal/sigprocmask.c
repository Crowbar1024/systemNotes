#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

// 设置某些信号是阻塞的，通过键盘产生这些信号，观察它们的未决状态打印到屏幕
int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);

    // 修改内核中的阻塞信号集，这样2、3号信号就被阻塞了
    sigprocmask(SIG_BLOCK, &set, NULL);

    int num = 0;
    while (1) {
        num++;
        // 获取当前的未决信号集的数据
        sigset_t pendingset;
        sigemptyset(&pendingset);
        sigpending(&pendingset);

        // 遍历前31位的常规信号
        for (int i = 1; i <= 31; i++) {
            if (sigismember(&pendingset, i) == 1) {
                printf("1");
            } else if (sigismember(&pendingset, i) == 0) {
                printf("0");
            } else {
                perror("sigismember");
                exit(0);
            }
        }
        printf("\n");
        
        sleep(1);
        if(num == 10) {
            // 解除阻塞
            sigprocmask(SIG_UNBLOCK, &set, NULL);
        }

    }

    return 0;
}
/*
    当信号没有产生时，自然未决信号集的状态都是0，当产生时，由于被阻塞了，所以未决状态显示1。
    直到10次循环后，解除了阻塞，信号才得以被处理，进程被终止。
*/ 