# 网络连接

## nc
全称netcat，用于设置路由器。它能通过 TCP 和 UDP 在网络中读写数据。
-l 使用监听模式，该参数意味着nc被当作server，侦听并接受连接。
-p<通信端口> 设置本地主机使用的通信端口。
-n 直接使用IP地址，而不通过域名服务器。

## dd
dd 可从标准输入或文件中读取数据，根据指定的格式来转换数据，再输出到文件、设备或标准输出。
if=文件名：输入文件名，默认为标准输入。
```bash
dd if=/dev/zero of=yhnb.txt bs=1M count=5  # 产生一个5M的空白文件
# 输出，可以用来测试文件写的速度
5+0 records in
5+0 records out
5242880 bytes (5.2 MB, 5.0 MiB) copied, 0.0061792 s, 848 MB/s
# 测试了多次，发现速度每次不一样
```


### 两个机器通过TCP通信
我用了一个wsl，一个虚拟机，都是ubuntu。发现WSL对某些命令的管理权限要求很高，而VM虚拟机不会这样
```bash
howie@yoga:~$ sudo nc -lp 333
yog@ubuntu:~/Desktop$ nc -n 172.18.157.201 333
# 然后可以随意通信
```
测试之后发现，howie关了，yog并没有立刻发现；反之会立刻发现；可能只是因为howie只是用来监听的吧

### 两台机器通过TCP传输数据
数据来源：/dev/zero。该文件是一个特殊的字符设备文件，当我们使用或者读取它的时候，它会提供无限连续不断的空数据流。
其中一个典型的用法就是用它提供的字符流来覆盖信息，另一个常见用法就是产生一个特定大小的空白文件。
这里使用第一种。

```bash
# p1端发送1000MB字节到p2端的5001端口 （nc 什么都不加就表示外部写数据到打开的地方）
p1: dd if=/dev/zero bs=1MB count=1000 | nc p2 5001  # 管道符将左边命令的输出变成右边命令的输入
p2: nc -l 5001 > /dev/null
126 MB/s  # 速度
```
由于是空字符串，所以算理论最快的速度，用来测TCP是最好的


### 两台机器通过TCP进行文件传输
把test.srt文件写到333端口（一切都是文件）
```bash
howie@yoga:~$ sudo nc -lp 333 < test.srt
yog@ubuntu:~/Desktop$ sudo nc -nv 172.18.157.201 333 > test.srt -p 1
```

### 本地文件传输1
开两个终端窗口，分别执行
```bash
p1: nc -l 5001 | pv -W > /dev/null  # pv -W 可以实时查看速率，但用的是1024，所以得换算一下
p2: dd if=/dev/zero bs=1MB count=5 |nc localhost 5001
473 MB/s  # 速度
```
这其实是开了四个进程：nc pv dd nc


### 本地文件传输2
dd毕竟是产生文件，那么用现有文件试试
```bash
p1: ls -lh asmr.wav  # 一个200M左右的文件
p1: time nc 172.27.226.164 5001 < asmr.wav
p2: nc -l 5001 > /dev/null
1072 MB/s
```
但我运行不了，当我想删除时，发现这些从windows复制过来的文件有写保护

### 速度不同的解释
两台机器通过TCP传输数据：118
本地dd传输数据：580
本地直接nc传输数据：1074

本地dd传输数据的解释：
/dev/zero和|管道和TCP在内核，dd和nc创建的进程在用户。
发送端先从/dev/zero读数据到dd，然后通过管道|，再到nc，nc再发到TCP，一共发生了四次用户和内核之间的拷贝。
接收端从TCP读数据到nc，nc再重定向写到/dev/null，发生了两次。
所以一共进行了6次。
所以真正的TCP->nc->/dev/null是显示速度的3倍。

本地直接nc传输数据的解释：
上面这一句速度更快的原因是，读文件到内存->nc->TCP->/dev/null是4次
```
