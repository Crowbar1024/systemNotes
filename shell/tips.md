# 小技巧

### 花括号自动扩展
```bash
$ echo {one,two,three}file
onefile twofile threefile

$ echo {one,two,three}{1,2,3}
one1 one2 one3 two1 two2 two3 three1 three2 three3

$ cp /very/long/path/file{,.bak}
# 给 file 复制一个叫做 file.bak 的副本

$ rm file{1,3,5}.txt
# 删除 file1.txt file3.txt file5.txt

$ mv *.{c,cpp} src/
# 将所有 .c 和 .cpp 为后缀的文件移入 src 文件夹
```

### 路径名称
```bash
$ pwd
/very/long/path
$ cd # ~ or home can be ignored
$ cd - # 返回并输出上一次所在的目录
```

$$ 记录当前进程的 PID。