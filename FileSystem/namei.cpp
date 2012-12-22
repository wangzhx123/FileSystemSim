#include "namei.h"
#include <bitset>
#include <string>
using namespace std;

// ����5������ find_first_zero, new_inode, free_inode, new_block, free_block, 
// �ᱻ����ĺ����õ�������Ҳ�Ͱ��������ˡ�=��=
template<size_t N> size_t find_first_zero(const bitset<N> &map) {
	// 0�������ʴ�1��ʼ
	for (size_t i = 1; i < map.size(); i++) {
		if (!map.test(i)) {
			return i;
		}
		else {
			continue;
		}
	}
	// ���û���ҵ�δ��λ��inode��ţ��򷵻�0
	return 0;
}

// �½�һ��i��㣬��λͼ���ҵ�δ�õ�inode����λ�󷵻���Ӧ��i���ṹָ�룬������ɹ�������NULL
CInode* new_inode(CFileSystem &fs) {
	size_t inode_num = 0;
	if ((inode_num = find_first_zero(fs.map_inode)) == 0) 
		return NULL;

	fs.map_inode.set(inode_num); // ��λ
	fs.inodes[inode_num].i_nlinks = 1; // �ļ���������1
	fs.inodes[inode_num].i_num = inode_num; // ��i����������¼����

	// ע�⣡�����ﲢû�и���i������block�ռ�,��i����i_zone[0]Ϊ0
	return &fs.inodes[inode_num];
}

STATUS free_inode(CFileSystem& fs, CInode &inode) {
	if (!fs.map_inode.test(inode.i_num)) 
		return FAILURE; // ��i���ԭ���Ͳ����ڣ���ʾ����
	
	fs.map_inode.reset(inode.i_num);
	// TODO ���ﻹҪ�����ͷ�i_zone[]������ĸ���block�Ĵ���
	return SUCCESS;
}

// ������һ��block����λͼ���ҵ�δ�õ�block��������Ӧ��block�ţ�������ɹ�������0
int new_block(CFileSystem &fs) {
	size_t block_num = 0;
	if ((block_num = find_first_zero(fs.map_block)) == 0) 
		return 0;

	fs.map_block.set(block_num); // ��λ
	fs.blocks[block_num].b_num = block_num; // �ڿ������м�¼������߼����

	return block_num;
}

STATUS free_block(CFileSystem &fs, CBlock &block) {
	if (!fs.map_block.test(block.b_num)) 
		return FAILURE; // ��blockԭ���Ͳ����ڣ���ʾ����
	
	fs.map_block.reset(block.b_num);
	return SUCCESS;
}

CNamei::CNamei(void) {}

CNamei::~CNamei(void) {}

// ��dir_base_inode��ʾ��Ŀ¼���в�����dir_nameƥ���Ŀ¼��ҵ�������inode�ţ�û���ҵ�����0
int CNamei::find_dir_entry(CFileSystem& fs, string dir_name, int dir_base_inode) {
	int num_of_blocks = fs.inodes[dir_base_inode].i_size / CBlock::SIZE_PER_BLOCK + 1;
	// TODO �������еĿ���������������Ӽ�Ҳ��û�п����ж���������Ρ�
	for (int i = 0; i < num_of_blocks; i++) {
		// ��i_zone�ֶ���õ��߼����
		size_t block_num = fs.inodes[dir_base_inode].i_zone[i];
		// ��Ŀ¼��Ϊ��λ����
		for (int j = 0; j < CBlock::SIZE_PER_BLOCK/16; j+=16) {
			DIR_ENTRY dir = fs.blocks[block_num].b_get_dir_entry(j);
			string temp(dir.d_name);
			if (dir_name.compare(temp) == 0) {
				// �����ҵ���i����
				return dir.d_inode;
			}
		}
	}
	// ���û���ҵ�������0
	return 0;
}

// �����ṩ��Ŀ¼��,��dir_inode��Ӧ��Ŀ¼���½�һ��Ŀ¼����ؽ��õ�Ŀ¼��inode�ţ�ʧ�ܷ���0
int CNamei::new_dir_entry(CFileSystem& fs, string dir_name, int dir_inode) {
	// ����һ��inode
	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 1; // ��������һ��Ŀ¼i���

	DIR_ENTRY dir_entry;
	dir_entry.d_inode = temp_inode->i_num;
	strncpy(dir_entry.d_name, dir_name.c_str(), FILENAME_LENGTH);

    // TODO ����Ӧ������дһ������������Ϊi������block�����ԭ�ռ䲻��������£�����Ϊ�����뵽��i���û��Ϊ�����block
	if(fs.inodes[dir_inode].i_size == 0) {
		// ˵������һ��֮ǰû�����ݵ�i��㣬�������ﻹҪ����Ϊ������һ��block�洢�ռ�
		fs.inodes[dir_inode].i_zone[0] = new_block(fs);
	}

	// �������һ����i_zone�����е�����ֵ
	int index = (0 + fs.inodes[dir_inode].i_size / CBlock::SIZE_PER_BLOCK);
	// �õ����һ��Ŀ¼���¼���ڵ��߼�����Լ����ڵ�ƫ�ƣ��Ա���������в����µ�Ŀ¼��
	size_t last_block = fs.inodes[dir_inode].i_zone[index];
	int offset = fs.inodes[dir_inode].i_size % CBlock::SIZE_PER_BLOCK;

	// �п������պ����ˣ��Ǿ�Ҫ������һ��i_zone���ݣ���¼������Ŀ��
	if (offset == 0) { 
		// TODO Ҫдһ���޸�i_zone�ĺ�������������Ĵ���������Ҳ�Ӽ�û�д�����
	}

	// ��block��offsetƫ�ƴ���ʼ���Ŀ¼��ṹ��Ϣ
	fs.blocks[last_block].b_put_dir_entry(dir_entry, offset);
	// �޸�i_size�ֶ�
	fs.inodes[dir_inode].i_size += 16; // TODO ������ò�Ҫ��16���Ӳ����
	return temp_inode->i_num;
}

// ��dir_baseĿ¼�в�����file_nameƥ����ļ����ҵ�������inode�ţ�û���ҵ�����0
int CNamei::find_file(CFileSystem& fs, string file_name, int dir_base_inode) {
	int num_of_blocks = fs.inodes[dir_base_inode].i_size / CBlock::SIZE_PER_BLOCK + 1;
	// TODO �������еĿ���������������ж������أ����ﻹû�п��ǣ������Ӧ�ö���дһ������
	for (int i = 0; i < num_of_blocks; i++) {
		// ��i_zone�ֶ���õ��߼����
		size_t block_num = fs.inodes[dir_base_inode].i_zone[i];
		// ��Ŀ¼��Ϊ��λ����
		for (int j = 0; j < CBlock::SIZE_PER_BLOCK/16; j+=16) {
			DIR_ENTRY dir = fs.blocks[block_num].b_get_dir_entry(j);
			string temp(dir.d_name);
			// TODO ע�⣡���ﲻ���ļ����ͣ�Ҳ����˵��һ��Ŀ¼�²�����ͬ����Ŀ¼���ļ�
			if (file_name.compare(temp) == 0) {
				return dir.d_inode;
			}
		}
	}
	// ���û���ҵ�
	return 0;
}
// ��dir_inode��Ӧ��Ŀ¼����ע��һ���ļ����½��������ļ�����Ϊ�գ�Ҳ����i_zone[0]Ϊ0�������ؽ��õ��ļ���inode�ţ�ʧ�ܷ���0
// ���ң�����򿪵�ģʽ�൱���ǡ�truncate��
int CNamei::open_file(CFileSystem& fs, string file_name, int dir_inode) {
	// ����һ��inode
	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 2; // ��������һ��Ŀ¼i���

	DIR_ENTRY dir_entry;
	dir_entry.d_inode = temp_inode->i_num;
	strncpy(dir_entry.d_name, file_name.c_str(), FILENAME_LENGTH);

    // TODO ����Ӧ������дһ������������Ϊi������block�����ԭ�ռ䲻��������£�����Ϊ�����뵽��i���û��Ϊ�����block
	if (fs.inodes[dir_inode].i_size == 0) {
		// ˵������һ��֮ǰû�����ݵ�i��㣬�������ﻹҪ����Ϊ������һ��block�洢�ռ�
		fs.inodes[dir_inode].i_zone[0] = new_block(fs);
	}

	// �������һ����i_zone�����е�����ֵ
	int index = (0 + fs.inodes[dir_inode].i_size / CBlock::SIZE_PER_BLOCK);
	// �õ����һ��Ŀ¼���¼���ڵ��߼�����Լ����ڵ�ƫ�ƣ��Ա���������в����µ�Ŀ¼��
	size_t last_block = fs.inodes[dir_inode].i_zone[index];
	int offset = fs.inodes[dir_inode].i_size % CBlock::SIZE_PER_BLOCK;

	// �п������պ����ˣ��Ǿ�Ҫ������һ��i_zone���ݣ���¼������Ŀ��
	if (offset == 0) { 
		// TODO Ҫдһ���޸�i_zone�ĺ�������������Ĵ���������Ҳ�Ӽ�û�д�����
	}

	// ��block��offsetƫ�ƴ���ʼ���Ŀ¼��ṹ��Ϣ
	fs.blocks[last_block].b_put_dir_entry(dir_entry, offset);
	// �޸�i_size�ֶ�
	fs.inodes[dir_inode].i_size += 16; // TODO ������ò�Ҫ��16���Ӳ����
	return temp_inode->i_num;
}

// TODO ��ֻ�Ǹ�demo�����Էǳ����������������ֻ����һ���հ��ļ���д�����1024���ֽڣ���count<=1024��
extern int file_write(CFileSystem &fs, CInode *inode, SFILE *filp, char *buf, int count);
struct SFILE;
int CNamei::write_file(CFileSystem& fs, int fd, char buf[], int count) {
	/*
	// ����ļ������ǿհ׵ģ������block���������ע��֪�����ǿ϶��ġ�
	if (fs.inodes[fd].i_size == 0) 
		fs.inodes[fd].i_zone[0] = new_block(fs);

	for (int i = 0; i < count; i++) {
		fs.blocks[fs.inodes[fd].i_zone[0]].b_put(buf[i], i);
	}

	fs.inodes[fd].i_size += count;
	// ����д����ֽ���
	return count;
	*/
	SFILE filp;
	filp.f_flags = 0x01; // APPEND
	filp.f_pos = 0;
	return file_write(fs, &(fs.inodes[fd]), &filp, buf, count);
}

int CNamei::read_file(CFileSystem& fs, int fd, char buf[], int count) {

}