#pragma once
#include "structsANDconstants.h"

struct DIR_ENTRY {
	unsigned short d_inode; // 4B
	char d_name[FILENAME_LENGTH]; // 12B
};

class CBlock
{
public:
	CBlock(void);
	~CBlock(void);

	int b_put(int byte, int b_pos);
	int b_get(int b_pos);
	int b_put_dir_entry(DIR_ENTRY dir_entry, int b_pos);
	DIR_ENTRY b_get_dir_entry(int b_pos);

public:
	static const size_t SIZE_PER_BLOCK = 1024;
	short b_num; // 逻辑块号
	char b_capacity[SIZE_PER_BLOCK];
	int size; // 记录当前b_capacity中实际使用的字节数
};
