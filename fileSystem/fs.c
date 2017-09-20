#include "global.h"
#include "ide.h"
#include "stdint.h"
#include "direct.h"
#include "stdio-kernel.h"
#include "memory.h"
#include "string.h"
#include "debug.h"
#include "super_block.h"
#include "inode.h"
#include "fs.h"
#include "list.h"

extern uint8_t channel_cnt;
extern struct ide_channel channels[2];
extern struct list partition_list;

struct partition *cur_part;


static void partition_format(struct partition *part)
{
	uint32_t boot_sector_sects = 1;
	uint32_t super_block_sects = 1;
	uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);
	uint32_t inode_table_sects = DIV_ROUND_UP(sizeof(struct inode) * MAX_FILES_PER_PART, SECTOR_SIZE);
	uint32_t used_sects = boot_sector_sects + super_block_sects + inode_table_sects + inode_bitmap_sects;
	uint32_t free_sects = part->sec_cnt - used_sects;

	uint32_t block_bitmap_sects;
	block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
	uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
	block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);

	struct super_block sb;
	sb.magic = 0x19590318;
	sb.sec_cnt = part->sec_cnt;
	sb.inode_cnt = MAX_FILES_PER_PART;
	sb.part_lba_base = part->start_lba;
	
	sb.block_bitmap_sects = block_bitmap_sects;
	sb.block_bitmap_lba = part->start_lba + 2;

	sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
	sb.inode_bitmap_sects = inode_bitmap_sects;

	sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
	sb.inode_table_sects = inode_bitmap_sects;

	sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
	sb.root_inode_no = 0;
	sb.dir_entry_size = sizeof(struct dir_entry);

	printk("%s info:\n", part->name);
	printk("magic: 0x%x\npart_lba_base: 0x%x\n \
			all_sectors: 0x%x\ninode_cnt: 0x%x\n \
			block_bitmap_lba: 0x%x\nblock_bitmap_sectors: 0x%x\n \
			inode_bitmap_lba: 0x%x\ninode_bitmap_sectors: 0x%x\n \
			inode_table_lba: 0x%x\ninode_table_sectors: 0x%x\n \
			data_start_lba: 0x%x\n", sb.magic, sb.part_lba_base, 
			sb.sec_cnt, sb.inode_cnt, sb.block_bitmap_lba, sb.block_bitmap_sects, 
			sb.inode_bitmap_lba, sb.inode_bitmap_sects, sb.inode_table_lba, 
			sb.inode_table_sects, sb.data_start_lba);

	struct disk *hd = part->my_disk;
	ide_write(hd, part->start_lba + 1, &sb, 1);
	printk("super_block_lba: 0x%x\n", part->start_lba + 1);

	uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects ? 
							sb.block_bitmap_sects : sb.inode_bitmap_sects);

	buf_size = (buf_size >= sb.inode_table_sects ?
					buf_size : sb.inode_table_sects) * SECTOR_SIZE;
	uint8_t *buf = (uint8_t *)sys_malloc(buf_size);

	buf[0] = 0x01;
	uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
	uint8_t block_bitmap_last_bit = block_bitmap_bit_len % 8;

	uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);
	memset(&buf[block_bitmap_last_byte], 0xff, last_size);

	uint8_t bit_idx = 0;
	while( bit_idx <= block_bitmap_last_bit ) {
		buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
	}
	ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

	memset(buf, 0, buf_size);
	buf[0] = 0x01;
	ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

	memset(buf, 0, buf_size);
	struct inode *i = (struct inode *)buf;
	i->i_size = sb.dir_entry_size * 2;
	i->i_no = 0;
	i->i_sectors[0] = sb.data_start_lba;
	ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

	memset(buf, 0, buf_size);
	struct dir_entry *p_de = (struct dir_entry *)buf;
	memcpy(p_de->name, ".", 1);
	p_de->i_no = 0;
	p_de->f_type = FT_DIRECTORY;
	p_de++;

	memcpy(p_de->name, "..", 2);
	p_de->i_no = 0;
	p_de->f_type = FT_DIRECTORY;

	ide_write(hd, sb.data_start_lba, buf, 1);

	printk(" root_dir_lba: 0x%x\n", sb.data_start_lba);
	printk("%s format done\n", part->name);
	sys_free(buf);
}


void filesys_init()
{
	uint8_t channel_no = 0, dev_no, part_idx = 0;
	
	struct super_block *sb_buf = (struct super_block *)sys_malloc(SECTOR_SIZE);
	if( sb_buf == NULL ) {
		PANIC("alloc to super_block point failed\n");
	}
	printk("searching filesystem.......\n");
	while( channel_no < channel_cnt ) {
		dev_no = 0;
		while( dev_no < 2 ) {
			if( dev_no == 0 ) {
				dev_no++;
				continue;
			}
			struct disk *hd = &channels[channel_no].devices[dev_no];
			struct partition *part = hd->prim_parts;
			while( part_idx < 12 ) {
				if( part_idx == 4 ) {
					part = hd->logic_parts;
				}
				if( part->sec_cnt != 0 ) {
					memset(sb_buf, 0, SECTOR_SIZE);
					ide_read(hd, part->start_lba + 1, sb_buf, 1);
					if( sb_buf->magic == 0x19590318 ) {
						printk("%s has filesystem\n", part->name);
					}
					else {
						printk("formatting %s's partition %s .......\n",hd->name, part->name);
						partition_format(part);
					}
				}
				part_idx++;
				part++;
			}
			dev_no++;
		}
		channel_no++;
	}
	char defaule_part[8] = "sdb1";
	list_traversal(&partition_list, mount_partition, (int)defaule_part);
	sys_free(sb_buf);
	open_root_dir(cur_part);
	uint32_t fd_idx = 0;
	while( fd_idx < MAX_FILE_OPEN ) {
		file_table[fd_idx++].fd_inode = NULL;
	}
}



static enum bool mount_partition(struct list_elem *pelem, int arg)
{
	char *part_name = (char *)arg;
	struct partition *part = elem2entry(struct partition, part_tag, pelem);
	if( strcmp(part->name, part_name) != 0 ) {
		return false;
	}
	cur_part = part;
	struct disk *hd = cur_part->my_disk;
	struct super_block *sb_buf = (struct super_block *)sys_malloc(SECTOR_SIZE);
	if( sb_buf == NULL )
		PANIC("malloc memory failed\n");
	cur_part->sb = (struct super_block *)sys_malloc(sizeof(struct super_block));
	if( cur_part->sb == NULL )
		PANIC("malloc memory failed\n");

	memset(sb_buf, 0, SECTOR_SIZE);
	ide_read(hd, cur_part->start_lba + 1, sb_buf, 1);
	memcpy(cur_part->sb, sb_buf, sizeof(struct super_block));

	cur_part->block_bitmap.bits = (uint8_t *)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);
	if( cur_part->block_bitmap.bits == NULL )
		PANIC("malloc memory failed\n");
	cur_part->block_bitmap.btmp_bytes_len = sb_buf->block_bitmap_sects * SECTOR_SIZE;
	ide_read(hd, sb_buf->block_bitmap_lba, cur_part->block_bitmap.bits, sb_buf->block_bitmap_sects);

	cur_part->inode_bitmap.bits = (uint8_t *)sys_malloc(sb_buf->inode_bitmap_sects * SECTOR_SIZE);
	if( cur_part->inode_bitmap.bits == NULL )
		PANIC("malloc memeory failed\n");
	cur_part->inode_bitmap.btmp_bytes_len = sb_buf->inode_bitmap_sects * SECTOR_SIZE;
	ide_read(hd, sb_buf->inode_bitmap_lba, cur_part->inode_bitmap.bits, sb_buf->inode_bitmap_sects);

	list_init(&cur_part->open_inodes);
	printk("mount %s done\n", part->name);
	return true;
}


static char *path_parse(char *pathname, char *name_host)
{
	if( pathname[0] == 0 )
		return NULL;
	if( pathname[0] == '/' ) {
		while( *(++pathname) == '/' );
	}
	while( *pathname != '/' && *pathname != 0 ) {
		*name_host++ = *pathname++;
	}
	return pathname;
}

int32_t path_depth_cnt(char *pathname)
{
	ASSERT(pathname != NULL);
	char *p = pathname;
	char name[MAX_FILE_NAME_LEN];

	uint32_t depth = 0;
	p = path_parse(p, name);
	while( name[0] ) {
		depth++;
		memset(name, 0, MAX_FILE_NAME_LEN);
		if( p ) {
			p = path_parse(p, name);
		}
	}
	return depth;
}


static int search_file(const char *pathname, struct path_depth_cnt *searched_record)
{
	if( !strcmp(pathname, "/") || !strcmp(pathname, "/.") || !strcmp(pathname, "/..") ) {
		searched_record->parent_dir = &root_dir;
		searched_record->file_type = FT_DIRECTORY;
		searched_record->searched_path[0] = 0;
		return 0;
	}
	uint32_t path_len = strlen(pathname);
	ASSERT(pathname[0] == '/' && path_len > 1 && path_len < MAX_PATH_LEN);
	char *sub_path = (char *)pathname;
	struct dir *parent_dir = &root_dir;
	struct dir_entry dir_e;

	char name[MAX_FILE_NAME_LEN] = {0};

	searched_record->parent_dir = parent_dir;
	searched_record->file_type = FT_UNKNOWN;
	uint32_t parent_inode_no = 0;

	sub_path = path_parse(sub_path, name);
	while( name[0] ) {
		ASSERT(strlen(searched_record->searched_path) < 512);
		strcat(searched_record->searched_path, "/");
		strcat(searched_record->searched_path, name);
		if( search_dir_entry(cur_part, parent_dir, name, &dir_e) ) {
			memset(name, 0, MAX_FILE_NAME_LEN);
			if( sub_path ) {
				sub_path = path_parse(sub_path, name);
			}
			if( dir_e.f_type == FT_DIRECTORY ) {
				parent_inode_no = parent_dir->inode->i_no;
				dir_close(parent_dir);
				parent_dir = dir_open(cur_part, dir_e.i_no);
				searched_record->parent_dir = parent_dir;
				continue;
			}
			else if( dir_e.f_type == FT_REGULAR ) {
				searched_record->file_type = FT_REGULAR;
				return dir_e.i_no;
			}
		}
		else {
			/**
			 *
			 */
			return -1;
		}
	}
	dir_close(searched_record->parent_dir);
	searched_record->parent_dir = dir_open(cur_part, parent_inode_no);
	searched_record->file_type = FT_DIRECTORY;
	return dir_e.i_no;
}

int32_t sys_open(const char *pathname, uint8_t flags)
{
	if( pathname[strlen(pathname) - 1] == '/' ) {
		printk("can't open a director\n");
		return -1;
	}
	ASSERT(flags <= 7);
	int32_t fd = -1;
	struct path_search_record searched_record;
	memset(&searched_record, 0, sizeof(struct path_search_record));
	uint32_t pathname_depth = path_depth_cnt((char *)pathname);
	int inode_no = search_file(pathname, &searched_record);
	enum bool found = inode_no != -1 ? true : false;
	if( searched_record.file_type == FT_DIRECTORY ) {
		printk("can't open a director\n");
		dir_close(searched_record);
		return -1;
	}
	uint32_t path_search_depth = path_depth_cnt(searched_record.searched_path);
	if( pathname_depth != path_search_depth ) {
		printk("cannot access %s: Not a director, subpath %s is't exist\n", pathname, searched_record.parent_dir);
		return -1;
	}
	if( !found && !(flags & O_CREAT) ) {
		printk("in path %s file %s is't exist\n", searched_record.searched_path, (strrchr(searched_record.searched_path, '/') + 1));
		dir_close(searched_record.parent_dir);
		return -1;
	}
	else if( found && (flags & O_CREAT)) {
		prink("%s has already exist!\n", pathname);
		dir_close(searched_record.parent_dir);
		return -1;
	}
	switch( flags & O_CREAT ) {
		case O_CREAT:
			printk("create file\n");
			fd = file_create(searched_record.parent_dir, (strrchr(pathname, '/') + 1), flags);
			dir_close(searched_record.parent_dir);
	}
	return fd;
}






























