#ifndef ARKOI_FS_EXT_EXT2_H
#define ARKOI_FS_EXT_EXT2_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ext_status {
	EXT_STATUS_OK = 0,
	EXT_STATUS_INVALID_ARGUMENT = 1,
	EXT_STATUS_IO_ERROR = 2,
	EXT_STATUS_BAD_MAGIC = 3,
	EXT_STATUS_UNSUPPORTED = 4,
	EXT_STATUS_OUT_OF_RANGE = 5
} ext_status;

typedef struct ext_device {
	void* context;
	int (*read)(void* context, uint64_t offset, void* data, size_t bytes);
} ext_device;

typedef struct ext_superblock {
	uint32_t inodes_count;
	uint32_t blocks_count;
	uint32_t reserved_blocks_count;
	uint32_t free_blocks_count;
	uint32_t free_inodes_count;
	uint32_t first_data_block;
	uint32_t log_block_size;
	uint32_t blocks_per_group;
	uint32_t inodes_per_group;
	uint16_t magic;
	uint32_t revision_level;
	uint16_t inode_size;
	uint32_t feature_compat;
	uint32_t feature_incompat;
	uint32_t feature_ro_compat;
} ext_superblock;

typedef struct ext_group_descriptor {
	uint32_t block_bitmap;
	uint32_t inode_bitmap;
	uint32_t inode_table;
	uint16_t free_blocks_count;
	uint16_t free_inodes_count;
	uint16_t used_dirs_count;
} ext_group_descriptor;

typedef struct ext_inode {
	uint16_t mode;
	uint16_t uid;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t mtime;
	uint32_t dtime;
	uint16_t gid;
	uint16_t links_count;
	uint32_t blocks;
	uint32_t flags;
	uint32_t block[15];
} ext_inode;

typedef struct ext_filesystem {
	ext_device device;
	ext_superblock superblock;
	uint32_t block_size;
	uint32_t inode_size;
	uint32_t group_count;
	uint64_t group_descriptor_table_offset;
} ext_filesystem;

ext_status ext2_mount(ext_filesystem* fs, ext_device device);

ext_status ext2_read_superblock(const ext_filesystem* fs, ext_superblock* superblock);

ext_status ext2_read_group_descriptor(const ext_filesystem* fs, uint32_t index, ext_group_descriptor* desc);

ext_status ext2_read_inode(const ext_filesystem* fs, uint32_t number, ext_inode* inode);

#ifdef __cplusplus
}
#endif

#endif
