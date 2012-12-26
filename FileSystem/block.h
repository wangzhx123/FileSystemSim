#pragma once
#include "structsANDconstants.h"

// �߼����п��ܷŵ��ǽṹ�������ݣ�����Ŀ¼��DIR_ENTRY������INDEX_ENTRY������������е�����ֵ��
struct DIR_ENTRY {
	unsigned short d_inode; // 2B
	char d_name[FILENAME_LENGTH]; // 14B
};
struct INDEX_ENTRY {
	// as big_endian, totally 2 Bytes for indices
	char high; // if high=0, then the biggest index only bound to 255 highest
	char low;
public:
	short get_index() {
		return low + (high << 8);
	}
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
	int b_put_index_entry(INDEX_ENTRY index_entry, int b_pos);
	INDEX_ENTRY b_get_index_entry(int file_block);

public:
	static const size_t SIZE_PER_BLOCK = 1024;
	short b_num; // �߼����
	char b_capacity[SIZE_PER_BLOCK];
	int size; // ��¼��ǰb_capacity��ʵ��ʹ�õ��ֽ���
};
