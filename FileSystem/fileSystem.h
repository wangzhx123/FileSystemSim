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
	S_BLOCK fs_s_block; // ������ 2O B
	std::bitset<INODE_NUM> map_inode; // i���λͼ 512b
	std::bitset<BLOCK_NUM> map_block; // �����߼���λͼ 1024 
	CInode inodes[INODE_NUM]; // i���� 512*sizeof(CInode)
	CBlock blocks[BLOCK_NUM]; // �����߼��� 1024*1024B
	// ������
	CNamei *namei;
	CBitmaps *bitmaps;
public:
	CFileSystem(void);
	~CFileSystem(void);
public:
	int mkdir(std::string dir, int dir_inode);
	// ��·������dir�����ļ�ϵͳ�н���Ŀ¼���ṹ
	int mkdir2(const char *dir_path);
	// �����ļ�������fd��ʵ�ʾ����ļ�i���ţ�
	int open(char *file_path);
	int write(int fd, char buf[], int count);
};
