#include "namei.h"
#include <bitset>
#include <string>
#include "namei2.h"
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

// dir_inode��Ҫ���Ŀ¼���Ŀ¼i���ţ�dir_name������ӵ�Ŀ¼����
// TODO ���������ENTRY������Ŀ¼�ˣ���i_type��������Ϊ��1
int CNamei::add_entry(CFileSystem& fs, string dir_name, int dir_inode) {
	if (dir_name.length() > FILENAME_LENGTH) // FILENAME_LENGHT=12
		dir_name = dir_name.substr(0, FILENAME_LENGTH);

	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 1; // ��������һ��Ŀ¼i���

	int block, i = 0;
	if ((block = fs.inodes[dir_inode].i_zone[0]) == 0) {
		// ���֮ǰ�Ǹ���Ŀ¼����δ����i_zone[0]��
		block = fs.inodes[dir_inode].i_zone[0] = create_block(fs, dir_inode, 0);
	}
	DIR_ENTRY *de = (DIR_ENTRY *)fs.blocks[block].b_capacity;
	while (1) {
		if ((char *)de >= fs.blocks[block].b_capacity + CBlock::SIZE_PER_BLOCK) {
			if ((block = create_block(fs, dir_inode, i/DIR_ENTRIES_PER_BLOCK)) == 0)
				return 0; // ʧ�ܷ���
			de = (DIR_ENTRY *)fs.blocks[block].b_capacity;
		}
		// TODO ˵�����Ŀ¼��û������ɾ�������µĿհ�Ŀ¼��
		if (i*sizeof(DIR_ENTRY) >= fs.inodes[dir_inode].i_size) {
			de->d_inode = 0;
			fs.inodes[dir_inode].i_size = (i+1)*sizeof(DIR_ENTRY);
		}
		// ���ܵ�ǰde��ָ��Ŀ¼���i����Ϊ0����������֮ǰɾ����mkdir�������µ�
		if (!de->d_inode) {
			DIR_ENTRY dir_entry;
			dir_entry.d_inode = temp_inode->i_num;
			strncpy(dir_entry.d_name, dir_name.c_str(), FILENAME_LENGTH);
			fs.blocks[block].b_put_dir_entry(dir_entry, (char *)de-fs.blocks[block].b_capacity);

			return temp_inode->i_num;
		}
		de++;
		i++;
	}
}
int CNamei::find_entry(CFileSystem& fs, string dir_name, int dir_base_inode) {
	int entries = fs.inodes[dir_base_inode].i_size / (sizeof(DIR_ENTRY));
	int block;
	if ((block = fs.inodes[dir_base_inode].i_zone[0]) == 0) {
		return 0;
	}
	int i = 0;
	DIR_ENTRY *de = (DIR_ENTRY *)fs.blocks[block].b_capacity;
	while (i < entries) {
		// ���һ��������꣬�������һ���߼���
		if ((char *)de >= fs.blocks[block].b_capacity + fs.blocks[block].size) {
			if ((block = bmap(fs, dir_base_inode, i/DIR_ENTRIES_PER_BLOCK)) == 0) {
				//	i += DIR_ENTRIES_PER_BLOCK;
				//	continue;
				return -1; // ����
			}
			de = (DIR_ENTRY *) fs.blocks[block].b_capacity;
			// �ڵ�ǰ�Ŀ��в���
		}
		if (string(de->d_name).compare(dir_name) == 0) {
			return de->d_inode;
		}
		de++;
		i++;
	}
	// û���ҵ�
	return 0;
}

// ��dir_inode��Ӧ��Ŀ¼����ע��һ���ļ����½��������ļ�����Ϊ�գ�Ҳ����i_zone[0]Ϊ0�������ؽ��õ��ļ���inode�ţ�ʧ�ܷ���0
// ���ң�����򿪵�ģʽ�൱���ǡ�truncate"
// TODO��������V0.2����Ҫ�ģ��й�Ŀ¼������ݣ���Ҫ��namei2.cpp��ͷ��������
// TODO ���д�ģʽ�����ٻ�Ҫ�������ǵ�ģʽ�ɡ���
int CNamei::open_file(CFileSystem& fs, string file_name, int dir_inode) {
	// ����һ��inode
	CInode *temp_inode;
	if((temp_inode = new_inode(fs)) == NULL)
		return 0;

	temp_inode->i_type = 2; // ��������һ���ļ�i���

	// Ϊ���ļ����������Ŀ¼��Ŀ¼��
	DIR_ENTRY dir_entry;
	dir_entry.d_inode = temp_inode->i_num;
	strncpy(dir_entry.d_name, file_name.c_str(), FILENAME_LENGTH);

    // �ⲽ�����ˣ���Ϊ��bmap����������صĺ�������
	//if (fs.inodes[dir_inode].i_size == 0) {
	//	// ˵������һ��֮ǰû�����ݵ�i��㣬�������ﻹҪ����Ϊ������һ��block�洢�ռ�
	//	fs.inodes[dir_inode].i_zone[0] = new_block(fs);
	//}

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

// TODO 
struct SFILE;
int CNamei::write_file(CFileSystem& fs, int fd, char buf[], int count) {
	SFILE filp;
	filp.f_flags = 0x01; // APPEND
	filp.f_pos = 0;
	return file_write(fs, &(fs.inodes[fd]), &filp, buf, count);
}

int CNamei::read_file(CFileSystem& fs, int fd, char buf[], int count) {
	SFILE filp;
	filp.f_flags = 0;
	filp.f_pos = 0;
	return file_read(fs, &(fs.inodes[fd]), &filp, buf, count);
}