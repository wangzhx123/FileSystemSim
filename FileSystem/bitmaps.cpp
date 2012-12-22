#include "Bitmaps.h"

CBitmaps::CBitmaps(void)
{
}

CBitmaps::~CBitmaps(void)
{
}

// ����5������ find_first_zero, new_inode, free_inode, new_block, free_block, 
// �ᱻ����ĺ����õ�������Ҳ�Ͱ��������ˡ�=��=
template<size_t N> size_t CBitmaps::find_first_zero(const bitset<N> &map) {
	// 0�������ʴ�1��ʼ
	for (size_t i = 1; i < map.size(); i++) {
		if (!map.test(i)) {
			return i;
		}
		else {
			continue;
		}
	}
	// ���û���ҵ�δ��λ��inode��ţ��򷵻�0
	return 0;
}

// �½�һ��i��㣬��λͼ���ҵ�δ�õ�inode����λ�󷵻���Ӧ��i���ṹָ�룬������ɹ�������NULL
CInode* CBitmaps::new_inode(CFileSystem &fs) {
	size_t inode_num = 0;
	if ((inode_num = find_first_zero(fs.map_inode)) == 0) 
		return NULL;

	fs.map_inode.set(inode_num); // ��λ
	fs.inodes[inode_num].i_nlinks = 1; // �ļ���������1
	fs.inodes[inode_num].i_num = inode_num; // ��i����������¼����

	// ע�⣡�����ﲢû�и���i������block�ռ�,��i����i_zone[0]Ϊ0
	return &fs.inodes[inode_num];
}

STATUS CBitmaps::free_inode(CFileSystem& fs, CInode &inode) {
	if (!fs.map_inode.test(inode.i_num)) 
		return FAILURE; // ��i���ԭ���Ͳ����ڣ���ʾ����
	
	fs.map_inode.reset(inode.i_num);
	// TODO ���ﻹҪ�����ͷ�i_zone[]������ĸ���block�Ĵ���
	return SUCCESS;
}

// ������һ��block����λͼ���ҵ�δ�õ�block��������Ӧ��block�ţ�������ɹ�������0
int CBitmaps::new_block(CFileSystem &fs) {
	size_t block_num = 0;
	if ((block_num = find_first_zero(fs.map_block)) == 0) 
		return 0;

	fs.map_block.set(block_num); // ��λ
	fs.blocks[block_num].b_num = block_num; // �ڿ������м�¼������߼����

	return block_num;
}

STATUS CBitmaps::free_block(CFileSystem &fs, CBlock &block) {
	if (!fs.map_block.test(block.b_num)) 
		return FAILURE; // ��blockԭ���Ͳ����ڣ���ʾ����
	
	fs.map_block.reset(block.b_num);
	return SUCCESS;
}
