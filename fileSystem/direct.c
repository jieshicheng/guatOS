#include "direct.h"
#include "ide.h"
#include "stdint.h"
#include "memory.h"
#include "global.h"
#include "fs.h"
#include "debug.h"
#include "string.h"
#include "inode.h"

struct dir root_dir;

void open_root_dir(struct partition *part)
{
	root_dir.inode = inode_open(part, part->sb->root_inode_no);
	root_dir.dir_pos = 0;
}

struct dir *dir_open(struct partition *part, uint32_t inode_no)
{
	struct dir *pdir = (struct dir *)sys_malloc(sizeof(struct dir));
	pdir->inode = inode_open(part, inode_no);
	pdir->dir_pos = 0;
	return pdir;
}

enum bool search_dir_entry(struct partition *part, struct dir *pdir, const char *name, struct dir *dir_e)
{
	uint32_t block_cnt = 140;
	uint32_t *all_blocks = (uint32_t *)sys_malloc(48 + 512);
	if( all_blocks == NULL ) {
		printk("malloc memory failed\n");
		return false;
	}
	uint32_t block_idx = 0;
	while( block_idx < 12 ) {
		all_blocks[block_idx] = pdir->inode->i_sectors[block_idx];
		block_idx++;
	}
	if( pdir->inode->i_sectors[12] != 0 ) {
		ide_read(part->my_disk, pdir->inode->i_sectors[12], all_blocks + 12, 1);
	}
	uint8_t *buf = (uint8_t *)sys_malloc(SECTOR_SIZE);
	struct dir_entry *p_de = (struct dir_entry *)buf;
	uint32_t dir_entry_size = part->sb->dir_entry_size;
	uint32_t dir_entry_cnt = SECTOR_SIZE / dir_entry_size;
	while( block_idx < block_cnt ) {
		if( all_blocks[block_idx] == 0 ) {
			block_idx++;
			continue;
		}
		ide_read(part->my_disk, all_blocks[block_idx], buf ,1);
		uint32_t dir_entry_idx = 0;
		while( dir_entry_idx < dir_entry_cnt ) {
			if( !strcmp(p_de->name, name) ) {
				memcpy(dir_e, p_de, dir_entry_size);
				sys_free(buf);
				sys_free(all_blocks);
				return true;
			}
			dir_entry_idx++;
			p_de++;
		}
		block_idx++;
		p_de = (struct dir_entry *)buf;
		memset(buf, 0, SECTOR_SIZE);
	}
	sys_free(buf);
	sys_free(all_blocks);
	return false;
}


void dir_close(struct dir *dir)
{
	if( dir == &root_dir )
		return;
	inode_close(dir->inode);
	sys_free(dir);
}

void create_dir_entry(char *name, uint32_t inode_no, uint8_t file_type, struct dir_entry *p_de)
{
	ASSERT(strlen(name) <= MAX_FILE_NAME_LEN);
	memcpy(p_de->name, name, strlen(name));
	p_de->i_no = inode_no;
	p_de->f_type = file_type;
}

enum bool sync_dir_entry()


























