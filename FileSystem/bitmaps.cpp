#include "Bitmaps.h"

CBitmaps::CBitmaps(void)
{
}

CBitmaps::~CBitmaps(void)
{
}

// 下面5个函数 find_first_zero, new_inode, free_inode, new_block, free_block, 
// 会被后面的函数用到，所以也就摆在这里了。=。=
template<size_t N> size_t CBitmaps::find_first_zero(const bitset<N> &map) {
	// 0保留，故从1开始
	for (size_t i = 1; i < map.size(); i++) {
		if (!map.test(i)) {
			return i;
		}
		else {
			continue;
		}
	}
	// 如果没有找到未置位的inode编号，则返回0
	return 0;
}

// 新建一个i结点，从位图中找到未用的inode，置位后返回相应的i结点结构指针，如果不成功，返回NULL
CInode* CBitmaps::new_inode(CFileSystem &fs) {
	size_t inode_num = 0;
	if ((inode_num = find_first_zero(fs.map_inode)) == 0) 
		return NULL;

	fs.map_inode.set(inode_num); // 置位
	fs.inodes[inode_num].i_nlinks = 1; // 文件链接数置1
	fs.inodes[inode_num].i_num = inode_num; // 在i结点属性里记录结点号

	// 注意！！这里并没有给该i结点分配block空间,即i结点的i_zone[0]为0
	return &fs.inodes[inode_num];
}

STATUS CBitmaps::free_inode(CFileSystem& fs, CInode &inode) {
	if (!fs.map_inode.test(inode.i_num)) 
		return FAILURE; // 该i结点原来就不存在，表示出错
	
	fs.map_inode.reset(inode.i_num);
	// TODO 这里还要加上释放i_zone[]数组里的各个block的代码
	return SUCCESS;
}

// 新申请一个block，从位图中找到未用的block，返回相应的block号，如果不成功，返回0
int CBitmaps::new_block(CFileSystem &fs) {
	size_t block_num = 0;
	if ((block_num = find_first_zero(fs.map_block)) == 0) 
		return 0;

	fs.map_block.set(block_num); // 置位
	fs.blocks[block_num].b_num = block_num; // 在块属性中记录本身的逻辑块号

	return block_num;
}

STATUS CBitmaps::free_block(CFileSystem &fs, CBlock &block) {
	if (!fs.map_block.test(block.b_num)) 
		return FAILURE; // 该block原来就不存在，表示出错
	
	fs.map_block.reset(block.b_num);
	return SUCCESS;
}
