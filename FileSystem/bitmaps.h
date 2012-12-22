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
	// �½�һ��i��㣬��λͼ���ҵ�δ�õ�inode����λ�󷵻���Ӧ��i���ṹָ�룬������ɹ�������NULL
	CInode* new_inode(CFileSystem &fs);
	// �ͷ�һ��i���
	STATUS free_inode(CFileSystem& fs, CInode &inode);
	// ������һ��block����λͼ���ҵ�δ�õ�block��������Ӧ��block�ţ�������ɹ�������0
	int new_block(CFileSystem &fs);
	// �ͷ�һ���߼���block
	STATUS free_block(CFileSystem &fs, CBlock &block);
};
