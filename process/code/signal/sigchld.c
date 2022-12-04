#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>



void myFun1(int num) {
    printf("catch signal %d\n", num);
    wait(NULL);
}

// 这样可以循环，可以回收掉所以僵尸进程，但阻塞的话不好。
void myFun2(int num) {
    printf("catch signal %d\n", num);
    while (1) {
        wait(NULL);
    }
}
// 误解：A子进程产生信号，调用了myfun函数，waitpid就只会去回收A进程。
// 解释：waitpid函数是个劳模，它只要见到僵尸进程就忍不住要回收，但能力有限，一次只能回收一次。
// 只要给它机会，它可以把所有的僵尸进程一网打尽。所以只要有while循环，就可以不断执行waitpid函数，直到break。



void myFun(int num) {
    printf("catch signal %d\n", num);
    while (1) {
       int ret = waitpid(-1, NULL, WNOHANG);  // 非阻塞回收所有子进程
       if (ret > 0) {
           printf("child die , pid = %d\n", ret);
       } else if(ret == 0) {  // 说明还有子进程活着，没有僵尸进程
           break;  // 跳出回调函数，父进程就可以继续执行了，继续等待其他子进程的信号。
       } else if(ret == -1) {  // 没有子进程
           break;
       }
    }
}
// 为什么先阻塞SIGCHLD信号，当注册完信号捕捉以后，再解除阻塞，这样就会继续执行回调函数回收资源？
// 提前阻塞的极端情况：20个子进程老早就终止了，内核收到SIGCHLD信号，会将未决信号集中的17号位置为1，
// 但由于信号被提前阻塞，阻塞信号集该位置是1，所以未决信号被阻塞，等待处理。
// 当父进程注册完信号捕捉函数以后，再解除阻塞，阻塞信号集该位置是0。
// 内核发现此时未决信号集第17号位居然是1，且没被阻塞，那么就去执行对应的捕获处理函数。
// 在处理函数中，waitpid函数回收所有僵尸进程。
// 综上，《Linux/UNIX系统编程手册》指出为了保障可移植性，应用应在创建任何子进程之前就设置信号捕捉函数。

int main() {

    // 因为有可能子进程很快结束，父进程还没有注册完信号捕捉，所以需要提前设置好阻塞信号集，阻塞SIGCHLD（17）。
    // 否则有可能产生段错误。
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // 创建子进程
    pid_t pid;
    for (int i = 0; i < 5; i++) {
        pid = fork();
        if (pid == 0) {
            break;
        }
    }
    if (pid > 0) {
        // 捕捉子进程死亡时发送的SIGCHLD信号
        struct sigaction act;
        act.sa_flags = 0;
        act.sa_handler = myFun1;
        sigemptyset(&act.sa_mask);
        sigaction(SIGCHLD, &act, NULL);

        // 注册完信号捕捉以后，解除阻塞
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        while (1) {
            printf("parent process pid : %d\n", getpid());
            sleep(2);
        }
    } else if (pid == 0) {
        printf("child process pid : %d\n", getpid());
    }

    return 0;
}


/*
    为什么某一次父进程捕获到了信号，进入了myfun函数，一个僵尸都没回收？

    Linux的信号机制：
        不排队。父进程在调用信号处理函数时，内核会自动堵塞同类型信号。
        (当然也可以用参数，自己指定要堵塞其他类型的信号。)
        要注意的是，这里堵塞不是不接收信号，而是接收了不处理。
        当父进程信号处理函数结束后，堵塞就会自动解除，该信号会传递给父进程。
    所以这是当父进程回收了所有僵尸进程后，正要回到主函数，内核把之前被堵塞的SIGCHLD信号又发送过来。
    父进程不得不继续进入信号处理函数，但此时已经没有子进程给他回收了。

    这个排队机制也说明了为什么myFun1不能全部回收，由于排队机制，最后传递的信号不全（小于子进程数）
    myFun1每次只能回收一个，那么调用次数不够，所以部分还依然是僵尸进程。


    段错误究竟是怎么发生的？段错误的复现为什么这么难？


    可重入函数的意思是：函数由两条或多条线程调用时，即便是交叉执行，其效果也与各线程以未定义顺序依次调用时一致。
    通俗点讲，就是存在一个函数，A线程执行一半，B线程抢过CPU又来调用该函数，执行到1/4倍A线程抢回执行权。
    在这样不断来回执行中，不出问题的，就是可重入函数。
    多线程中每个线程都有自己的堆栈，所以如果函数中只用到局部变量肯定是可重入的。
    但是更新了全局变量或静态数据结构的函数可能是不可重入的。
    假设某线程正在为一个链表结构添加一个新的链表项，而另外一个线程也视图更新同一链表。
    由于中间涉及多个指针，一旦另一线程中断这些步骤并修改了相同指针，结果就会产生混乱。
    但是并不是一定会出现，一定是A线程刚好在修改指针，另外一线程又去修改才会出现。
    这就是为什么该问题复现难度较高的原因。

    将静态数据结构用于内部记账的函数也是不可重入的。
    其中最明显的例子就是stdio函数库成员（printf()、scanf()等），它们会为缓冲区I/O更新内部数据结构。
    所以，如果在捕捉信号处理函数中调用了printf()，而主程序又在调用printf()，
    或其他stdio函数期间遭到了捕捉信号处理函数的中断，那么有时就会看到奇怪的输出，设置导致程序崩溃。
    虽然printf()不是异步信号安全函数，但却频频出现在各种示例中，是因为在展示对捕捉信号处理函数的调用，
    以及显示函数中相关变量的内容时，printf()都不失为一种简单而又便捷的方式。真正的应用程序应当避免使用该类函数。
    printf函数会使用到一块缓冲区，这块缓冲区是使用malloc或类似函数分配的一块静态内存。所以它是不可重入函数。
*/