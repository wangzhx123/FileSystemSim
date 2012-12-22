#include "namei.h"
#include <bitset>
#include <string>
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

// 在dir_base_inode表示的目录的中查找与dir_name匹配的目录项，找到返回其inode号，没有找到返回0
int CNamei::find_dir_entry(CFileSystem& fs, string dir_name, int dir_base_inode) {
	int num_of_blocks = fs.inodes[dir_base_inode].i_size / CBlock::SIZE_PER_BLOCK + 1;
	// TODO 在其所有的块中搜索，但这里从简也就没有考虑有二级块的情形。
	for (int i = 0; i < num_of_blocks; i++) {
		// 从i_zone字段里得到逻辑块号
		size_t block_num = fs.inodes[dir_base_inode].i_zone[i];
		// 以目录项为单位查找
		for (int j = 0; j < CBlock::SIZE_PER_BLOCK/16; j+=16) {
			DIR_ENTRY dir = fs.blocks[block_num].b_get_dir_entry(j);
			string temp(dir.d_name);
			if (dir_name.compare(temp) == 0) {
				// 返回找到的i结点号
				return dir.d_inode;
			}
		}
	}
	// 如果没有找到，返回0
	return 0;
}

// 根据提供的目录名,在dir_inode对应的目录下新建一个目录项，返回建好的目录的inode号，失败返回0
int CNamei::new_dir_entry(CFileSystem& fs, string dir_name, int dir_inode) {
	// 申请一个inode
	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 1; // 表明这是一个目录i结点

	DIR_ENTRY dir_entry;
	dir_entry.d_inode = temp_inode->i_num;
	strncpy(dir_entry.d_name, dir_name.c_str(), FILENAME_LENGTH);

    // TODO 这里应该另外写一个函数，负责为i结点分配block（如果原空间不够的情况下），因为该申请到的i结点没有为其分配block
	if(fs.inodes[dir_inode].i_size == 0) {
		// 说明这是一个之前没有内容的i结点，所以这里还要负责为它申请一个block存储空间
		fs.inodes[dir_inode].i_zone[0] = new_block(fs);
	}

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

// 在dir_base目录中查找与file_name匹配的文件，找到返回其inode号，没有找到返回0
int CNamei::find_file(CFileSystem& fs, string file_name, int dir_base_inode) {
	int num_of_blocks = fs.inodes[dir_base_inode].i_size / CBlock::SIZE_PER_BLOCK + 1;
	// TODO 在其所有的块中搜索，但如果有二级块呢（这里还没有考虑）？因此应该独立写一个函数
	for (int i = 0; i < num_of_blocks; i++) {
		// 从i_zone字段里得到逻辑块号
		size_t block_num = fs.inodes[dir_base_inode].i_zone[i];
		// 以目录项为单位查找
		for (int j = 0; j < CBlock::SIZE_PER_BLOCK/16; j+=16) {
			DIR_ENTRY dir = fs.blocks[block_num].b_get_dir_entry(j);
			string temp(dir.d_name);
			// TODO 注意！这里不看文件类型，也就是说，一个目录下不能有同名的目录和文件
			if (file_name.compare(temp) == 0) {
				return dir.d_inode;
			}
		}
	}
	// 如果没有找到
	return 0;
}
// 在dir_inode对应的目录项中注册一个文件（新建），该文件内容为空（也就是i_zone[0]为0）。返回建好的文件的inode号，失败返回0
// 而且，这里打开的模式相当于是“truncate”
int CNamei::open_file(CFileSystem& fs, string file_name, int dir_inode) {
	// 申请一个inode
	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 2; // 表明这是一个目录i结点

	DIR_ENTRY dir_entry;
	dir_entry.d_inode = temp_inode->i_num;
	strncpy(dir_entry.d_name, file_name.c_str(), FILENAME_LENGTH);

    // TODO 这里应该另外写一个函数，负责为i结点分配block（如果原空间不够的情况下），因为该申请到的i结点没有为其分配block
	if (fs.inodes[dir_inode].i_size == 0) {
		// 说明这是一个之前没有内容的i结点，所以这里还要负责为它申请一个block存储空间
		fs.inodes[dir_inode].i_zone[0] = new_block(fs);
	}

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

// TODO 这只是个demo，所以非常不完整。这个函数只能往一个空白文件里写入最多1024个字节（即count<=1024）
extern int file_write(CFileSystem &fs, CInode *inode, SFILE *filp, char *buf, int count);
struct SFILE;
int CNamei::write_file(CFileSystem& fs, int fd, char buf[], int count) {
	/*
	// 如果文件内容是空白的，则分配block。由上面的注释知，这是肯定的。
	if (fs.inodes[fd].i_size == 0) 
		fs.inodes[fd].i_zone[0] = new_block(fs);

	for (int i = 0; i < count; i++) {
		fs.blocks[fs.inodes[fd].i_zone[0]].b_put(buf[i], i);
	}

	fs.inodes[fd].i_size += count;
	// 返回写入的字节数
	return count;
	*/
	SFILE filp;
	filp.f_flags = 0x01; // APPEND
	filp.f_pos = 0;
	return file_write(fs, &(fs.inodes[fd]), &filp, buf, count);
}

int CNamei::read_file(CFileSystem& fs, int fd, char buf[], int count) {

}