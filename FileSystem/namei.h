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
	//--�����ж�������ȫ�ֵ�FileSystem���󣬴��л��bitmap��inode�Լ�block����Ϣ--//
	// ��dir_base��block�в�����dir_nameƥ���Ŀ¼��ҵ�������i���ţ�û���ҵ�����0
	int find_dir_entry(CFileSystem& fs, std::string dir_name, int dir_base_inode);
	// ��dir_baseĿ¼�в�����file_nameƥ����ļ����ҵ�������i���ţ�û���ҵ�����0
	int find_file(CFileSystem& fs, std::string file_name, int dir_base);
	// �����ṩ��Ŀ¼��,��dir_inode��Ӧ��Ŀ¼���½�һ��Ŀ¼����ؽ��õ�Ŀ¼��i���ţ�ʧ�ܷ���0
	int new_dir_entry(CFileSystem& fs, std::string dir_name, int dir_inode);
	// �����ṩ���ļ���,��dir_inode��Ӧ��Ŀ¼����ע��һ���ļ�����ؽ��õ��ļ���i���ţ�ʧ�ܷ���0
	int open_file(CFileSystem& fs, std::string file_name, int dir_inode);
	//--TODO �����Ƕ��ļ���������ֻ�Ǹ�demo�������԰���������������ȻӦ������дһ��ģ��--//
	// ��buf������ݣ�д����file_inodeΪi���ŵ��ļ�block��
	int write_file(CFileSystem& fs, int file_inode, char buf[], int count);
	int read_file(CFileSystem& fs, int file_inode, char buf[], int count);
};

