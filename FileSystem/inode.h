#pragma once

class CFileSystem;
enum STATUS;

class CInode
{
public:
	short i_num; // i结点号
	short i_type; // 1代表目录，2代表文件
	short i_size; // 文件当前大小
	short i_nlinks; // 链接到该文件的个数
	short i_zone[9]; // 该文件的数据块的磁盘逻辑块号
public:
	CInode(void);
	~CInode(void);
	CInode* iget(CFileSystem& fs, size_t inode_num);
	STATUS iput(CFileSystem& fs, CInode& inode);
};
