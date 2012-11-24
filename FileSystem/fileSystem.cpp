#include "fileSystem.h"
#include "namei.h"
#include "cstring"

using namespace std;

CFileSystem::CFileSystem() {
	this->namei = new CNamei();
	// 0�Ų��ã�0��Ϊʧ�ܱ�ǣ���Ϊ�˲�����find_first_zero��������ʼ����λΪ1
	this->map_block.set(0);
	this->map_inode.set(0);
	// ��ʼ������㣨/������Ϣ
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
	// ���û���ҵ�����ֱ�Ӵ���һ��
	int dir_inode_found = namei->find_dir_entry(*this, dir, dir_inode);
	if (dir_inode_found == 0) {
		return namei->new_dir_entry(*this, dir, dir_inode);
	} else {
		return dir_inode_found;
	}
}

int CFileSystem::mkdir2(const char *dir_path) {	
	// pch ����Ϊ������dir_path�ϵĸ���Ŀ¼����һ�����Һͽ���
	char *pch;
	pch = strtok(const_cast<char *>(dir_path), "/");
	int dir_inode_found = 0;
	// �涨dir_pathһ��Ҫ��һ�����Ե�ַ�����ԴӸ�Ŀ¼��ʼ
	int base_dir_inode = 1; 
	while(pch != NULL) {
		// �Ӹ�Ŀ¼��ʼ��
		dir_inode_found = namei->find_dir_entry(*this, string(pch), base_dir_inode);
		if (dir_inode_found == 0) {
			// ���û����Ӧ��Ŀ¼�������Ͻ���һ��
			base_dir_inode = namei->new_dir_entry(*this, string(pch), base_dir_inode);
		} else {
			// ����ҵ�����Ӧ��Ŀ¼���͸��µ�ǰ����Ŀ¼����������
			base_dir_inode = dir_inode_found;
		}
		// ����pchΪ��һ��Ŀ¼��
		pch = strtok(NULL, "/");
	}
	// ��󷵻ؽ��õ�Ŀ¼�Ľ���
	return base_dir_inode;
}

int CFileSystem::open(char *file_path) {
	// �Ƚ��ļ�·�����ֽ�õ�Ŀ¼���ļ�������
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
	
	// ����Ŀ¼�����û�о��½�һ��������Ŀ¼��
	int dir_inode = mkdir2(dir);
	// �����ļ������û�о��½�һ���հ��ļ�����󷵻��ļ�i����
	int file_inode = namei->find_file(*this, string(file_name), dir_inode);
	if (file_inode == 0)
		return namei->open_file(*this, string(file_name), dir_inode);
	else
		return file_inode;
}

int CFileSystem::write(int fd, char buf[], int count) {
	return namei->write_file(*this, fd, buf, count);
}
