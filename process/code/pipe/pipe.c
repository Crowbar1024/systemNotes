#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    // 在fork之前创建管道
    int pipefd[2];
    int ret = pipe(pipefd);
    if(ret == -1) {
        perror("pipe");
        exit(0);
    }

    pid_t pid = fork();
    if (pid > 0) {
        printf("i am parent process, pid : %d\n", getpid());
        close(pipefd[1]);  // 关闭写端
        char buf[1024] = {0};
        // 可以设置非阻塞，这样如果管道为空，读端进行读，就会返回-1，继续执行下去
        // int flags = fcntl(pipefd[0], F_GETFL);
        // flags |= O_NONBLOCK;
        // fcntl(pipefd[0], F_SETFL, flags);
        int len = read(pipefd[0], buf, sizeof(buf));  // 从管道的读取端读取数据
        printf("parent recv : %s, pid : %d\n", buf, getpid());
        wait(NULL);
    } else if (pid == 0){
        printf("i am child process, pid : %d\n", getpid());
        close(pipefd[0]);  // 关闭读端

        // 可以调用其他进程
        // dup2(fd[1], STDOUT_FILENO);  // 重定向stdout到管道写端
        // execlp("ps", "ps", "aux", NULL);
        // perror("execlp");
        // exit(0);

        // 向父进程写数据
        char buf[108];
        int times = 6;
        while (times--) {
            // 用stdin作为输入
            // memset(buf, 0, 128);
            // fgets(buf, 128, stdin);
            // write(pipefd[1], buf, strlen(buf));

            char* str = "hello,i am child";
            write(pipefd[1], str, strlen(str));  // 向管道中写入数据
        }
    } else {
        perror("fork");
        exit(0);
    }
    return 0;
}


