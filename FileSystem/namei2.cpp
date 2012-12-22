#include "fileSystem.h"
#include "namei.h"
#include "bitmaps.h"
#include <iostream>
using namespace std;

// ���createΪtrue,���ڲ�����ӳ���ʱ���½�
// file_blockΪ�ļ��Ķ�д��ţ�ÿ���ļ���file_block�����Ҷ��Ǵ�0��ʼ
// ��������file_blockת��Ϊ���������Ӵ������������ݵ��߼����
static int _bmap(CFileSystem &fs, int inode_num, int file_block, bool create) {
	if (file_block < 0) {
		cerr << "ERROR: file_block number < 0" << endl;
		return 0;
	}
	// ��Ϊi_zone[0]-i_zone[6]Ϊһ���飬i_zone[7]Ϊ�����飬i_zone[8]Ϊ������
	// ��ÿ���������п��Դ��512���߼���ţ�һ���߼���Ϊ2���ֽڣ�
	if (file_block >= 7 + 512 + 512*512){
		cerr << "ERROR: it's out of the max_file_size" << endl;
		return 0;
	}

	// CInode inode = fs.inodes[inode_num];
	// ˵�������ҵ��߼������һ������
	if (file_block < 7) {
		// �����ؿ�δ���䣬��createΪTRUE�����·���һ�����ݿ�
		if (create && !fs.inodes[inode_num].i_zone[file_block]) {
			if (fs.inodes[inode_num].i_zone[file_block] = fs.bitmaps->new_block(fs))
				return fs.inodes[inode_num].i_zone[file_block];
			else { 
				cerr << "ERROR: fail to allocate a disk block!" << endl;
				return 0;
			}
		}
		// ���createΪfalse���򷵻�0��ʾδ�ڴ������ҵ���Ӧ���߼���
		else
			return 0;
	}
	// ��������ҵ��߼�����ڶ�������
	file_block -= 7;
	if (file_block < 512) {
		// ���i_zone[7]��δʹ��
		if (create && !fs.inodes[inode_num].i_zone[7]) {
			if (fs.inodes[inode_num].i_zone[7] = fs.bitmaps->new_block(fs)) {
				// �������ɹ������ȡ������������д�������߼����
				INDEX_ENTRY index_entry;
				// TODO ����ֻд��low����������ܳ���255(not any more...but need to be tested)
				unsigned int temp;
				if (temp = fs.bitmaps->new_block(fs)) {
					index_entry.low = temp & 0x0000FFFF;
					index_entry.high = temp >> 8;
					fs.blocks[fs.inodes[inode_num].i_zone[7]].b_put_index_entry(index_entry, 0);
					return (int)index_entry.low;
				}
				else { 
					cerr << "ERROR: fail to allocate a disk block!" << endl;
					return 0; 
				}
			}
			else { 
				cerr << "ERROR: fail to allocate a disk block!" << endl;
				return 0; 
			}
		}
		// ���createΪfalse���򷵻�0��ʾδ�ڴ������ҵ���Ӧ���߼���
		else
			return 0;
	}

	// ˵�������ҵ��߼��������������
	// Ϊ�˼�࣬����ʡ����һЩ������Դ���
	file_block -= 512;
	if (file_block < 512*512) {
		if (create && !fs.inodes[inode_num].i_zone[8]) {
			if (fs.inodes[inode_num].i_zone[8] = fs.bitmaps->new_block(fs)) {
				// �������ɹ������ȡ������������д�����Ķ������
				INDEX_ENTRY index_entry2;
				// TODO ����ֻд��low����������ܳ���255
				if (index_entry2.low = fs.bitmaps->new_block(fs)) {
					fs.blocks[fs.inodes[inode_num].i_zone[8]].b_put_index_entry(index_entry2, 0);
					// �������ɹ������ȡ������������д������һ����ֱ�ӣ��߼����
					INDEX_ENTRY index_entry1;
					// TODO ����ֻд��low����������ܳ���255
					if (index_entry1.low = fs.bitmaps->new_block(fs)) {
						fs.blocks[(int)index_entry2.low].b_put_index_entry(index_entry1, 0);
						return (int)index_entry1.low;
					}
				}
			}
			else {
				cerr << "ERROR: fail to allocate a disk block!" << endl;
				return 0; 
			}
		}
		// ���createΪfalse���򷵻�0��ʾδ�ڴ������ҵ���Ӧ���߼���
		else
			return 0;
	}
	// will never come to this point
	return 0;
}

static int create_block(CFileSystem &fs, int inode_num, int file_block) {
	return _bmap(fs, inode_num, file_block, true);
}
static int bmap(CFileSystem &fs, int inode_num, int file_block) {
	return _bmap(fs, inode_num, file_block, false);
}


// TODO 
#define O_APPEND 0x01
// count Ϊ��Ҫд����ֽ���
int file_write(CFileSystem &fs, CInode *inode, SFILE *filp, char *buf, int count) {
	// pos Ϊ��д����ļ�ƫ��ָ��
	off_t pos;
	if (filp->f_flags & O_APPEND)
		pos = inode->i_size;
	else
		pos = filp->f_pos;

	// iΪ����ǰ���ѭ��Ϊֹ�ۼ���д����ֽ���
	// cΪ��ǰ���ѭ����д����ֽ���
	int i = 0, c, block_num;
	while (i < count) {
		// һ��ѭ����͸���д������һ�����ݿ飬���δд�꣬�ͽ�����һ��ѭ��
		if (!(block_num = create_block(fs, inode->i_num, pos/CBlock::SIZE_PER_BLOCK)))
			break;
		c = pos % CBlock::SIZE_PER_BLOCK;
		int b_pos = c; // Ҫд��λ�����ڵ����ݿ�ƫ��
		// �����ٽ�c����Ϊ�����ݿ���ʣ���д���ֽ���
		c = CBlock::SIZE_PER_BLOCK - c;
		// |xxxxxxx(pos)<----(c)---->| (1 block, '-' :been wriiten; '_':not yet been written)
		if (c > count-i) // ˵��������ݿ鲻��д�����⽫��Ҫд�����һ��
			c = count-i; // ���ǽ�c����Ϊʵ��Ҫ�������ݿ���д����ֽ���

		// �޸�pos!Ϊ�˺�����ȷ��λ�ļ��µĶ�дλ��
		pos += c;
		if (pos > inode->i_size) {
			// TODO ��Ȼ�ᵽ�����޸��ļ���С, so why there is an if?
			inode->i_size = pos;
		}
		i += c;
		while (c-- > 0)
			fs.blocks[block_num].b_put(*(buf++), b_pos++);
	}

	// ����FILE�ṹ����ļ���дָ��λ��f_pos
	if (!(filp->f_flags & O_APPEND)) {
		filp->f_pos = pos;
	}
	// ���շ���ʵ��д����ֽ���
	return i ? i : -1;
}

inline int find_min(int a, int b) {
	if (a < b) return a;
	else return b;
}
// TODO
int file_read(CFileSystem &fs, CInode *inode, FILE *filp, char *buf, int count) {
	int left;
	int chars, offset, block_num, byte_read;
	if ((left = count) <= 0)
		return 0;

	// ѭ���������ݣ�ֱ������������ֽ�������û��������Ϊֹ
	while (left) {
		if (offset = bmap(fs, inode, (filp->f_pos)/BLOCK_SIZE) {
			// ������Ӧ���߼���
			block_num = offset;
			// �ٽ�offset����Ϊ�ļ���дλ�õĿ���ƫ��
			offset = filp->f_pos % CBlock::SIZE_PER_BLOCK;
			// �Ƚϸÿ���ʣ��ɶ����ֽ����뻹Ҫ�����ֽ���left�Ĵ�С����Сֵ��Ϊ��δӸ����ݿ��ж�ȡ���ֽ���chars
			chars = find_min(CBlock::SIZE_PER_BLOCK-offset, left);
			// �����ļ���дλ��ָ��
			filp->f_pos += chars;
			// ���½�������Ҫ�����ֽ���
			left -= chars;
			// �����ݿ�������ֽڶ�����buf��
			while (chars-- > 0) {
				if((byte_read = fs.blocks[block_num].b_get(offset++)) != -1)
					*(buf++) = (char)read_byte;
				else 
					// ������ݿ��е�����ֻ�в��ֿɶ�������count�ֽڣ���buf�������0���
					// TODO but lack efficiency here...
					*(buf++) = 0;
				}
			}
		}
		else {
			// ���bmap��������0����ʾû���ڴ������ҵ���Ӧ���߼��飨���ļ����ݸ���û����count��ô�ࣩ
			return (count-left) ? (count-left) : -1;
		}
	}

	return (count-left) ? (count-left) : -1;
}
