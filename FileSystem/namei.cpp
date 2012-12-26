#include "namei.h"
#include <bitset>
#include <string>
#include "namei2.h"
using namespace std;

// 下面5个函数 find_first_zero, new_inode, free_inode, new_block, free_block, 
// 会被后面的函数用到，所以也就摆在这里了。=。=
template<size_t N> size_t find_first_zero(const bitset<N> &map) {
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
CInode* new_inode(CFileSystem &fs) {
	size_t inode_num = 0;
	if ((inode_num = find_first_zero(fs.map_inode)) == 0) 
		return NULL;

	fs.map_inode.set(inode_num); // 置位
	fs.inodes[inode_num].i_nlinks = 1; // 文件链接数置1
	fs.inodes[inode_num].i_num = inode_num; // 在i结点属性里记录结点号

	// 注意！！这里并没有给该i结点分配block空间,即i结点的i_zone[0]为0
	return &fs.inodes[inode_num];
}

STATUS free_inode(CFileSystem& fs, CInode &inode) {
	if (!fs.map_inode.test(inode.i_num)) 
		return FAILURE; // 该i结点原来就不存在，表示出错
	
	fs.map_inode.reset(inode.i_num);
	// TODO 这里还要加上释放i_zone[]数组里的各个block的代码
	return SUCCESS;
}

// 新申请一个block，从位图中找到未用的block，返回相应的block号，如果不成功，返回0
int new_block(CFileSystem &fs) {
	size_t block_num = 0;
	if ((block_num = find_first_zero(fs.map_block)) == 0) 
		return 0;

	fs.map_block.set(block_num); // 置位
	fs.blocks[block_num].b_num = block_num; // 在块属性中记录本身的逻辑块号

	return block_num;
}

STATUS free_block(CFileSystem &fs, CBlock &block) {
	if (!fs.map_block.test(block.b_num)) 
		return FAILURE; // 该block原来就不存在，表示出错
	
	fs.map_block.reset(block.b_num);
	return SUCCESS;
}

CNamei::CNamei(void) {}
CNamei::~CNamei(void) {}

// dir_inode是要添加目录项的目录i结点号，dir_name是欲添加的目录项名
// TODO 这里把所有ENTRY都当作目录了，即i_type都被设置为了1
int CNamei::add_entry(CFileSystem& fs, string dir_name, int dir_inode) {
	if (dir_name.length() > FILENAME_LENGTH) // FILENAME_LENGHT=12
		dir_name = dir_name.substr(0, FILENAME_LENGTH);

	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 1; // 表明这是一个目录i结点

	int block, i = 0;
	if ((block = fs.inodes[dir_inode].i_zone[0]) == 0) {
		// 如果之前是个空目录（即未分配i_zone[0]）
		block = fs.inodes[dir_inode].i_zone[0] = create_block(fs, dir_inode, 0);
	}
	DIR_ENTRY *de = (DIR_ENTRY *)fs.blocks[block].b_capacity;
	while (1) {
		if ((char *)de >= fs.blocks[block].b_capacity + CBlock::SIZE_PER_BLOCK) {
			if ((block = create_block(fs, dir_inode, i/DIR_ENTRIES_PER_BLOCK)) == 0)
				return 0; // 失败返回
			de = (DIR_ENTRY *)fs.blocks[block].b_capacity;
		}
		// TODO 说明这个目录下没有由于删除而留下的空白目录项
		if (i*sizeof(DIR_ENTRY) >= fs.inodes[dir_inode].i_size) {
			de->d_inode = 0;
			fs.inodes[dir_inode].i_size = (i+1)*sizeof(DIR_ENTRY);
		}
		// 可能当前de所指的目录项的i结点号为0，即可能是之前删除（mkdir）而留下的
		if (!de->d_inode) {
			DIR_ENTRY dir_entry;
			dir_entry.d_inode = temp_inode->i_num;
			strncpy(dir_entry.d_name, dir_name.c_str(), FILENAME_LENGTH);
			fs.blocks[block].b_put_dir_entry(dir_entry, (char *)de-fs.blocks[block].b_capacity);

			return temp_inode->i_num;
		}
		de++;
		i++;
	}
}
int CNamei::find_entry(CFileSystem& fs, string dir_name, int dir_base_inode) {
	int entries = fs.inodes[dir_base_inode].i_size / (sizeof(DIR_ENTRY));
	int block;
	if ((block = fs.inodes[dir_base_inode].i_zone[0]) == 0) {
		return 0;
	}
	int i = 0;
	DIR_ENTRY *de = (DIR_ENTRY *)fs.blocks[block].b_capacity;
	while (i < entries) {
		// 如果一个块查找完，则读入下一个逻辑块
		if ((char *)de >= fs.blocks[block].b_capacity + fs.blocks[block].size) {
			if ((block = bmap(fs, dir_base_inode, i/DIR_ENTRIES_PER_BLOCK)) == 0) {
				//	i += DIR_ENTRIES_PER_BLOCK;
				//	continue;
				return -1; // 出错
			}
			de = (DIR_ENTRY *) fs.blocks[block].b_capacity;
			// 在当前的块中查找
		}
		if (string(de->d_name).compare(dir_name) == 0) {
			return de->d_inode;
		}
		de++;
		i++;
	}
	// 没有找到
	return 0;
}

// 在dir_inode对应的目录项中注册一个文件（新建），该文件内容为空（也就是i_zone[0]为0）。返回建好的文件的inode号，失败返回0
// 而且，这里打开的模式相当于是“truncate"
// TODO！！！！V0.2这里要改！有关目录项的内容，都要按namei2.cpp里头的那样改
// TODO 还有打开模式，至少还要个不覆盖的模式吧……
int CNamei::open_file(CFileSystem& fs, string file_name, int dir_inode) {
	// 申请一个inode
	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 2; // 表明这是一个文件i结点

	// 为该文件填充它所在目录的目录项
	DIR_ENTRY dir_entry;
	dir_entry.d_inode = temp_inode->i_num;
	strncpy(dir_entry.d_name, file_name.c_str(), FILENAME_LENGTH);

    // 这步不用了，因为在bmap函数中有相关的函数操作
	//if (fs.inodes[dir_inode].i_size == 0) {
	//	// 说明这是一个之前没有内容的i结点，所以这里还要负责为它申请一个block存储空间
	//	fs.inodes[dir_inode].i_zone[0] = new_block(fs);
	//}

	// 计算最后一块在i_zone数组中的索引值
	int index = (0 + fs.inodes[dir_inode].i_size / CBlock::SIZE_PER_BLOCK);
	// 得到最后一个目录项记录所在的逻辑块号以及块内的偏移，以便继续在其中插入新的目录项
	size_t last_block = fs.inodes[dir_inode].i_zone[index];
	int offset = fs.inodes[dir_inode].i_size % CBlock::SIZE_PER_BLOCK;

	// 有可能这块刚好满了，那就要新增加一个i_zone数据，记录新申请的块号
	if (offset == 0) { 
		// TODO 要写一个修改i_zone的函数来进行这里的处理，但这里也从简没有处理了
	}

	// 在block的offset偏移处开始添加目录项结构信息
	fs.blocks[last_block].b_put_dir_entry(dir_entry, offset);
	// 修改i_size字段
	fs.inodes[dir_inode].i_size += 16; // TODO 这里最好不要用16这个硬编码
	return temp_inode->i_num;
}

// TODO 
struct SFILE;
int CNamei::write_file(CFileSystem& fs, int fd, char buf[], int count) {
	SFILE filp;
	filp.f_flags = 0x01; // APPEND
	filp.f_pos = 0;
	return file_write(fs, &(fs.inodes[fd]), &filp, buf, count);
}

int CNamei::read_file(CFileSystem& fs, int fd, char buf[], int count) {
	SFILE filp;
	filp.f_flags = 0;
	filp.f_pos = 0;
	return file_read(fs, &(fs.inodes[fd]), &filp, buf, count);
}