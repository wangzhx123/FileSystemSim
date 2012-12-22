#include "block.h"
#include <algorithm>

CBlock::CBlock()
{
	this->size = 0;
}

CBlock::~CBlock()
{
}

// 在块在b_pos(<1024)处写入一个byte数据
int CBlock::b_put(int byte, int b_pos) {
	if (b_pos == SIZE_PER_BLOCK) 
		return 0; // 若该磁盘块已满，返回0

	b_capacity[b_pos] = (char) byte;
	this->size++;
	return 1; // return the number of bytes been put
}

// 从b_pos偏移处读一个byte返回
int CBlock::b_get(int b_pos) {
	// 如果该块为空，返回-1
	if (this->size == 0)
		return -1;
	if (b_pos <= this->size)
		return b_capacity[b_pos];
	else
		return -1; // 说明读出的是文件内容不存在的字节
}

DIR_ENTRY CBlock::b_get_dir_entry(int b_pos) {
	// 如果块中剩下的字节数不足16字节（一个目录项长度）
	if (SIZE_PER_BLOCK - b_pos < 16) 
		exit(-1); // because this should never happen!

	char temp[16];
	for (int i = 0; i < 16; i++) {
		temp[i] = b_get(b_pos++);
	}

	DIR_ENTRY d_temp;
	d_temp.d_inode = temp[0]; // int->char
	std::copy(temp+4, temp+16, d_temp.d_name);

	return d_temp;
}

int CBlock::b_put_dir_entry(DIR_ENTRY dir_entry, int b_pos) {
	if (b_pos == SIZE_PER_BLOCK)
		return -1;

	// 写入i结点号 (4B，实际上只有一个字节大小)
	b_capacity[b_pos] = (char) dir_entry.d_inode;
	b_pos += 4;
	// 写入文件名 (12B)
	for (int i = 0; i < FILENAME_LENGTH; i++) {
		b_capacity[b_pos++] = dir_entry.d_name[i];
	}

	int bytes_read = 4 + FILENAME_LENGTH;
	this->size += bytes_read;
	return bytes_read;
}

int CBlock::b_put_index_entry(INDEX_ENTRY index_entry, int b_pos) {
	if (b_pos == SIZE_PER_BLOCK)
		return -1;

	// 写入索引号的高8位
	b_capacity[b_pos] = (char) index_entry.high;
	b_pos ++;
	// 再写入索引号的低8位
	b_capacity[b_pos] = (char) index_entry.low;

	int bytes_read = 2;
	this->size += bytes_read;
	return bytes_read;
}
INDEX_ENTRY CBlock::b_get_index_entry(int b_pos) {
	// 如果块中剩下的字节数不足2字节（一个索引项长度）
	if (SIZE_PER_BLOCK - b_pos < 2) 
		exit(-1); // because this should never happen!

	INDEX_ENTRY i_temp;
	i_temp.high = this->b_get(b_pos);
	i_temp.high = this->b_get(b_pos + 1);
	return i_temp;
}