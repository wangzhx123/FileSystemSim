#pragma once
#include <bitset>
#include "block.h"
#include "inode.h"
#include "namei.h"
#include "bitmaps.h"
#include "structsANDconstants.h"

class CInode;
class CBlock;
class CNamei;
class CBitmaps;

class CFileSystem
{
public:
	S_BLOCK fs_s_block; // 超级块 2O B
	std::bitset<INODE_NUM> map_inode; // i结点位图 512b
	std::bitset<BLOCK_NUM> map_block; // 磁盘逻辑块位图 1024 
	CInode inodes[INODE_NUM]; // i结点块 512*sizeof(CInode)
	CBlock blocks[BLOCK_NUM]; // 磁盘逻辑块 1024*1024B
	// 工具类
	CNamei *namei;
	CBitmaps *bitmaps;
public:
	CFileSystem(void);
	~CFileSystem(void);
public:
	int mkdir(std::string dir, int dir_inode);
	// 按路径名字dir，在文件系统中建立目录树结构
	int mkdir2(const char *dir_path);
	// 返回文件描述符fd（实际就是文件i结点号）
	int open(char *file_path);
	int write(int fd, char buf[], int count);
};
