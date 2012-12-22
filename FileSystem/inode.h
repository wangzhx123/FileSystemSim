#pragma once

class CFileSystem;
enum STATUS;

class CInode
{
public:
	short i_num; // i����
	short i_type; // 1����Ŀ¼��2�����ļ�
	short i_size; // �ļ���ǰ��С
	short i_nlinks; // ���ӵ����ļ��ĸ���
	short i_zone[9]; // ���ļ������ݿ�Ĵ����߼����
public:
	CInode(void);
	~CInode(void);
	CInode* iget(CFileSystem& fs, size_t inode_num);
	STATUS iput(CFileSystem& fs, CInode& inode);
};
