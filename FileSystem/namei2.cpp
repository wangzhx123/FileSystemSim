#include "fileSystem.h"
#include "namei.h"
#include "bitmaps.h"
#include <iostream>
using namespace std;

// 如果create为true,则在不存在映射的时候新建
// file_block为文件的读写块号，每个文件的file_block独立且都是从0开始
// 函数将把file_block转换为可以用来从磁盘上索引数据的逻辑块号
static int _bmap(CFileSystem &fs, int inode_num, int file_block, bool create) {
	if (file_block < 0) {
		cerr << "ERROR: file_block number < 0" << endl;
		return 0;
	}
	// 因为i_zone[0]-i_zone[6]为一级块，i_zone[7]为二级块，i_zone[8]为三级块
	// 且每个二级块中可以存放512个逻辑块号（一个逻辑号为2个字节）
	if (file_block >= 7 + 512 + 512*512){
		cerr << "ERROR: it's out of the max_file_size" << endl;
		return 0;
	}

	// CInode inode = fs.inodes[inode_num];
	// 说明待查找的逻辑块号在一级块中
	if (file_block < 7) {
		// 如果相关块未分配，且create为TRUE，则新分配一个数据块
		if (create && !fs.inodes[inode_num].i_zone[file_block]) {
			if (fs.inodes[inode_num].i_zone[file_block] = fs.bitmaps->new_block(fs))
				return fs.inodes[inode_num].i_zone[file_block];
			else { 
				cerr << "ERROR: fail to allocate a disk block!" << endl;
				return 0;
			}
		}
		// 如果create为false，则返回0表示未在磁盘上找到相应的逻辑块
		else
			return 0;
	}
	// 如果待查找的逻辑块号在二级块中
	file_block -= 7;
	if (file_block < 512) {
		// 如果i_zone[7]尚未使用
		if (create && !fs.inodes[inode_num].i_zone[7]) {
			if (fs.inodes[inode_num].i_zone[7] = fs.bitmaps->new_block(fs)) {
				// 如果分配成功，则读取二级块内容以写入分配的逻辑块号
				INDEX_ENTRY index_entry;
				// TODO 这里只写入low，所以最大不能超过255(not any more...but need to be tested)
				unsigned int temp;
				if (temp = fs.bitmaps->new_block(fs)) {
					index_entry.low = temp & 0x0000FFFF;
					index_entry.high = temp >> 8;
					fs.blocks[fs.inodes[inode_num].i_zone[7]].b_put_index_entry(index_entry, 0);
					return (int)index_entry.low;
				}
				else { 
					cerr << "ERROR: fail to allocate a disk block!" << endl;
					return 0; 
				}
			}
			else { 
				cerr << "ERROR: fail to allocate a disk block!" << endl;
				return 0; 
			}
		}
		// 如果create为false，则返回0表示未在磁盘上找到相应的逻辑块
		else
			return 0;
	}

	// 说明待查找的逻辑块号在三级块中
	// 为了简洁，这里省略了一些错误调试代码
	file_block -= 512;
	if (file_block < 512*512) {
		if (create && !fs.inodes[inode_num].i_zone[8]) {
			if (fs.inodes[inode_num].i_zone[8] = fs.bitmaps->new_block(fs)) {
				// 如果分配成功，则读取三级块内容以写入分配的二级块号
				INDEX_ENTRY index_entry2;
				// TODO 这里只写入low，所以最大不能超过255
				if (index_entry2.low = fs.bitmaps->new_block(fs)) {
					fs.blocks[fs.inodes[inode_num].i_zone[8]].b_put_index_entry(index_entry2, 0);
					// 如果分配成功，则读取二级块内容以写入分配的一级（直接）逻辑块号
					INDEX_ENTRY index_entry1;
					// TODO 这里只写入low，所以最大不能超过255
					if (index_entry1.low = fs.bitmaps->new_block(fs)) {
						fs.blocks[(int)index_entry2.low].b_put_index_entry(index_entry1, 0);
						return (int)index_entry1.low;
					}
				}
			}
			else {
				cerr << "ERROR: fail to allocate a disk block!" << endl;
				return 0; 
			}
		}
		// 如果create为false，则返回0表示未在磁盘上找到相应的逻辑块
		else
			return 0;
	}
	// will never come to this point
	return 0;
}

static int create_block(CFileSystem &fs, int inode_num, int file_block) {
	return _bmap(fs, inode_num, file_block, true);
}
static int bmap(CFileSystem &fs, int inode_num, int file_block) {
	return _bmap(fs, inode_num, file_block, false);
}


// TODO 
#define O_APPEND 0x01
// count 为将要写入的字节数
int file_write(CFileSystem &fs, CInode *inode, SFILE *filp, char *buf, int count) {
	// pos 为待写入的文件偏移指针
	off_t pos;
	if (filp->f_flags & O_APPEND)
		pos = inode->i_size;
	else
		pos = filp->f_pos;

	// i为到当前这次循环为止累计已写入的字节数
	// c为当前这次循环里写入的字节数
	int i = 0, c, block_num;
	while (i < count) {
		// 一个循环里就负责写（满）一个数据块，如果未写完，就进入下一个循环
		if (!(block_num = create_block(fs, inode->i_num, pos/CBlock::SIZE_PER_BLOCK)))
			break;
		c = pos % CBlock::SIZE_PER_BLOCK;
		int b_pos = c; // 要写入位置所在的数据块偏移
		// 下面再将c设置为该数据块中剩余可写的字节数
		c = CBlock::SIZE_PER_BLOCK - c;
		// |xxxxxxx(pos)<----(c)---->| (1 block, '-' :been wriiten; '_':not yet been written)
		if (c > count-i) // 说明这个数据块不会写满，这将是要写的最后一块
			c = count-i; // 于是将c设置为实际要往该数据块里写入的字节数

		// 修改pos!为了后面正确定位文件新的读写位置
		pos += c;
		if (pos > inode->i_size) {
			// TODO 显然会到这里修改文件大小, so why there is an if?
			inode->i_size = pos;
		}
		i += c;
		while (c-- > 0)
			fs.blocks[block_num].b_put(*(buf++), b_pos++);
	}

	// 更新FILE结构里的文件读写指针位置f_pos
	if (!(filp->f_flags & O_APPEND)) {
		filp->f_pos = pos;
	}
	// 最终返回实际写入的字节数
	return i ? i : -1;
}

inline int find_min(int a, int b) {
	if (a < b) return a;
	else return b;
}
// TODO
int file_read(CFileSystem &fs, CInode *inode, FILE *filp, char *buf, int count) {
	int left;
	int chars, offset, block_num, byte_read;
	if ((left = count) <= 0)
		return 0;

	// 循环读出数据，直到读出请求的字节数或者没有数据了为止
	while (left) {
		if (offset = bmap(fs, inode, (filp->f_pos)/BLOCK_SIZE) {
			// 读出相应的逻辑块
			block_num = offset;
			// 再将offset设置为文件读写位置的块内偏移
			offset = filp->f_pos % CBlock::SIZE_PER_BLOCK;
			// 比较该块中剩余可读的字节数与还要读的字节数left的大小，较小值者为这次从该数据块中读取的字节数chars
			chars = find_min(CBlock::SIZE_PER_BLOCK-offset, left);
			// 更新文件读写位置指针
			filp->f_pos += chars;
			// 更新接下来还要读的字节数
			left -= chars;
			// 将数据块请求的字节读出到buf中
			while (chars-- > 0) {
				if((byte_read = fs.blocks[block_num].b_get(offset++)) != -1)
					*(buf++) = (char)read_byte;
				else 
					// 如果数据块中的数据只有部分可读但不足count字节，则buf后面就用0填充
					// TODO but lack efficiency here...
					*(buf++) = 0;
				}
			}
		}
		else {
			// 如果bmap函数返回0，表示没有在磁盘上找到相应的逻辑块（即文件内容根本没那有count那么多）
			return (count-left) ? (count-left) : -1;
		}
	}

	return (count-left) ? (count-left) : -1;
}
