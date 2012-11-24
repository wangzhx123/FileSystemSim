#include "inode.h"
#include "fileSystem.h"

CInode::CInode(void)
{
	this->i_num = 0;
	this->i_size = 0;
	this->i_nlinks = 0;
	this->i_zone[0] = 0;
}

CInode::~CInode(void)
{
}

CInode* CInode::iget(CFileSystem& fs, size_t inode_num) {
	if (inode_num >= INODE_NUM || !fs.map_inode.test(inode_num))
		return NULL;

	return &(fs.inodes[inode_num]);
}

STATUS CInode::iput(CFileSystem& fs, CInode &inode) {
	fs.inodes[inode.i_num] = inode; // TODO
	return SUCCESS;
}
