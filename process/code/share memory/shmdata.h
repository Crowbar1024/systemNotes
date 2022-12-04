#ifndef _SHMDATA_H_HEADER
#define _SHMDATA_H_HEADER
 
#define TEXT_SZ 2048
 
struct shared_use_st
{
	char text[TEXT_SZ];  // 记录写入和读取的文本
    int m_flag;
};

enum flag {
    canRead = 0,
    canWrite
};

#endif