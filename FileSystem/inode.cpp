#include "inode.h"
#include "fileSystem.h"

CInode::CInode(void)
{
	this->i_num = 0;
	this->i_size = 0;
	this->i_nlinks = 0;
	for (int i = 0; i < 9; i++) {
		this->i_zone[i] = 0;
	}
}

CInode::~CInode(void)
{
}

CInode* CInode::iget(CFileSystem& fs, size_t inode_num) {
	if (inode_num >= INODE_NUM || !fs.map_inode.test(inode_num))
		return NULL;

	return &(fs.inodes[inode_num]);
}

// 这个函数貌似没用到过。。都直接写了。。以后要改用这个函数实现保存inode的功能
STATUS CInode::iput(CFileSystem& fs, CInode &inode) {
	fs.inodes[inode.i_num] = inode; 
	return SUCCESS;
}
