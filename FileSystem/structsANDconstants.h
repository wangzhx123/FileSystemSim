#pragma once

const int FILENAME_LENGTH = 12;
enum STATUS {
	SUCCESS = 1,
	FAILURE = 0
};
static const size_t INODE_NUM = 100;
static const size_t BLOCK_NUM = 512;
struct S_BLOCK {
	short s_inodes; // sizeof(short) = 2 Bytes
	short s_nzones;
	short s_imap_blocks;
	short s_zmap_blocks;
	short s_firstdatazone;
	short s_log_zone_size;
	int s_max_size;
	short s_magic;
}; // 超级块结构体

struct SFILE {
	int f_pos; // 文件读写位置偏移量
	char f_flags; // 文件读写模式
};