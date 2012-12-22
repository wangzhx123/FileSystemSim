#pragma once
#include "fileSystem.h"
#include "block.h"
#include <string>

class CNamei
{
public:
	CNamei();
	~CNamei();
public:
	//--函数中都传入了全局的FileSystem对象，从中获得bitmap、inode以及block等信息--//
	// 在dir_base的block中查找与dir_name匹配的目录项，找到返回其i结点号，没有找到返回0
	int find_dir_entry(CFileSystem& fs, std::string dir_name, int dir_base_inode);
	// 在dir_base目录中查找与file_name匹配的文件，找到返回其i结点号，没有找到返回0
	int find_file(CFileSystem& fs, std::string file_name, int dir_base);
	// 根据提供的目录名,在dir_inode对应的目录下新建一个目录项，返回建好的目录的i结点号，失败返回0
	int new_dir_entry(CFileSystem& fs, std::string dir_name, int dir_inode);
	// 根据提供的文件名,在dir_inode对应的目录项中注册一个文件项，返回建好的文件的i结点号，失败返回0
	int open_file(CFileSystem& fs, std::string file_name, int dir_inode);
	//--TODO 下面是读文件函数，这只是个demo程序，所以把它放在这个类里，不然应该另外写一个模块--//
	// 将buf里的内容，写入以file_inode为i结点号的文件block中
	int write_file(CFileSystem& fs, int file_inode, char buf[], int count);
	int read_file(CFileSystem& fs, int file_inode, char buf[], int count);
};

