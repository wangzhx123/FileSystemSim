#include "fileSystem.h"
#include "namei.h"
#include "cstring"

using namespace std;

CFileSystem::CFileSystem() {
	this->namei = new CNamei();
	// 0号不用（0作为失败标记），为了不干扰find_first_zero函数，初始化置位为1
	this->map_block.set(0);
	this->map_inode.set(0);
	// 初始化根结点（/）的信息
	this->map_inode.set(1);
	this->inodes[1].i_num = 1;
	this->inodes[1].i_type = 1;
	this->inodes[1].i_nlinks = 1;
	this->inodes[1].i_size = 0;
}

CFileSystem::~CFileSystem() {
	free(namei);
}

int CFileSystem::mkdir(std::string dir, int dir_inode) {
	// 如果没有找到，就直接创建一个
	int dir_inode_found = namei->find_dir_entry(*this, dir, dir_inode);
	if (dir_inode_found == 0) {
		return namei->new_dir_entry(*this, dir, dir_inode);
	} else {
		return dir_inode_found;
	}
}

int CFileSystem::mkdir2(const char *dir_path) {	
	// pch 依次为给定的dir_path上的各个目录名，一层层查找和建立
	char *pch;
	pch = strtok(const_cast<char *>(dir_path), "/");
	int dir_inode_found = 0;
	// 规定dir_path一定要是一个绝对地址，所以从根目录开始
	int base_dir_inode = 1; 
	while(pch != NULL) {
		// 从根目录开始找
		dir_inode_found = namei->find_dir_entry(*this, string(pch), base_dir_inode);
		if (dir_inode_found == 0) {
			// 如果没有相应的目录，就马上建立一个
			base_dir_inode = namei->new_dir_entry(*this, string(pch), base_dir_inode);
		} else {
			// 如果找到了相应的目录，就更新当前查找目录，继续查找
			base_dir_inode = dir_inode_found;
		}
		// 更新pch为下一个目录名
		pch = strtok(NULL, "/");
	}
	// 最后返回建好的目录的结点号
	return base_dir_inode;
}

int CFileSystem::open(char *file_path) {
	// 先将文件路径名分解得到目录与文件两部分
	string str(file_path);
	size_t found = str.find_last_of("/");
	/*
	const char *dir = str.substr(0, found).c_str();
	const char *file_name = str.substr(found+1).c_str();
	*/
	string t = str.substr(0, found);
	char *dir = new char[t.size() + 1];
	strcpy(dir, t.c_str());
	t = str.substr(found+1);
	char *file_name = new char[t.size() + 1];
	strcpy(file_name, t.c_str());
	
	// 查找目录，如果没有就新建一个，返回目录号
	int dir_inode = mkdir2(dir);
	// 查找文件，如果没有就新建一个空白文件，最后返回文件i结点号
	int file_inode = namei->find_file(*this, string(file_name), dir_inode);
	if (file_inode == 0)
		return namei->open_file(*this, string(file_name), dir_inode);
	else
		return file_inode;
}

int CFileSystem::write(int fd, char buf[], int count) {
	return namei->write_file(*this, fd, buf, count);
}
