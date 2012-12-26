#pragma once
static int _bmap(CFileSystem &fs, int inode_num, int file_block, bool create);
int create_block(CFileSystem &fs, int inode_num, int file_block);
int bmap(CFileSystem &fs, int inode_num, int file_block);

int file_write(CFileSystem &fs, CInode *inode, SFILE *filp, char *buf, int count);
inline int find_min(int a, int b);
int file_read(CFileSystem &fs, CInode *inode, SFILE *filp, char *buf, int count);