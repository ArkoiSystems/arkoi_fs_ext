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
	uint32_t s_inodes_count;         /**< Total inode count. */
	uint32_t s_blocks_count;         /**< Total block count. */
	uint32_t s_r_blocks_count;       /**< Reserved block count. */
	uint32_t s_free_blocks_count;    /**< Free block count. */
	uint32_t s_free_inodes_count;    /**< Free inode count. */
	uint32_t s_first_data_block;     /**< First data block number. */
	uint32_t s_log_block_size;       /**< Block size as log2 shift from 1024. */
	uint32_t s_log_frag_size;        /**< Fragment size as log2 shift from 1024. */
	uint32_t s_blocks_per_group;     /**< Blocks per block group. */
	uint32_t s_frags_per_group;      /**< Fragments per block group. */
	uint32_t s_inodes_per_group;     /**< Inodes per block group. */
	uint32_t s_mtime;                /**< Last mount time (Unix time). */
	uint32_t s_wtime;                /**< Last write time (Unix time). */
	uint16_t s_mnt_count;            /**< Mount count since last check. */
	uint16_t s_max_mnt_count;        /**< Max mounts before forced check. */
	uint16_t s_magic;                /**< EXT2 signature, must be 0xEF53. */
	uint16_t s_state;                /**< Filesystem state flags. */
	uint16_t s_errors;               /**< Error handling behavior. */
	uint16_t s_minor_rev_level;      /**< Minor revision level. */
	uint32_t s_lastcheck;            /**< Last consistency check time. */
	uint32_t s_checkinterval;        /**< Max interval between checks. */
	uint32_t s_creator_os;           /**< Creator OS identifier. */
	uint32_t s_rev_level;            /**< Revision level. */
	uint16_t s_def_resuid;           /**< Default UID for reserved blocks. */
	uint16_t s_def_resgid;           /**< Default GID for reserved blocks. */
	uint32_t s_first_ino;            /**< First non-reserved inode. */
	uint16_t s_inode_size;           /**< Size of each inode structure. */
	uint16_t s_block_group_nr;       /**< Block group number of this copy. */
	uint32_t s_feature_compat;       /**< Compatible feature flags. */
	uint32_t s_feature_incompat;     /**< Incompatible feature flags. */
	uint32_t s_feature_ro_compat;    /**< Read-only compatible feature flags. */
	uint8_t s_uuid[16];              /**< Filesystem UUID. */
	char s_volume_name[16];          /**< Volume name (not null-terminated). */
	char s_last_mounted[64];         /**< Last mount path. */
	uint32_t s_algo_bitmap;          /**< Compression algorithm bitmap. */
	uint8_t s_prealloc_blocks;       /**< File preallocation blocks hint. */
	uint8_t s_prealloc_dir_blocks;   /**< Directory preallocation blocks hint. */
	uint8_t s_padding_1[2];          /**< Reserved padding. */
	uint8_t s_journal_uuid[16];      /**< Journal UUID (for ext3/ext4). */
	uint32_t s_journal_inum;         /**< Journal inode number. */
	uint32_t s_journal_dev;          /**< Journal device number. */
	uint32_t s_last_orphan;          /**< Head of orphan inode list. */
	uint32_t s_hash_seed[4];         /**< HTREE hash seed. */
	uint8_t s_def_hash_version;      /**< Default directory hash version. */
	uint8_t s_padding_2[3];          /**< Reserved padding. */
	uint32_t s_default_mount_opts;   /**< Default mount options. */
	uint32_t s_first_meta_bg;        /**< First metablock group. */
	uint8_t s_padding_3[760];        /**< Remaining superblock bytes to 1024. */
} ext_superblock;

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

#ifdef __cplusplus
}
#endif

#endif
