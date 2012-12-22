#pragma once
#include "namei.h"
#include <bitset>
#include <string>
using namespace std;

class CBitmaps
{
public:
	CBitmaps(void);
	~CBitmaps(void);
private:
	template<size_t N> size_t find_first_zero(const bitset<N> &map);
public:
	// 新建一个i结点，从位图中找到未用的inode，置位后返回相应的i结点结构指针，如果不成功，返回NULL
	CInode* new_inode(CFileSystem &fs);
	// 释放一个i结点
	STATUS free_inode(CFileSystem& fs, CInode &inode);
	// 新申请一个block，从位图中找到未用的block，返回相应的block号，如果不成功，返回0
	int new_block(CFileSystem &fs);
	// 释放一个逻辑块block
	STATUS free_block(CFileSystem &fs, CBlock &block);
};
