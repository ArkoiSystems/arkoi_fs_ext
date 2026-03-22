#ifndef ARKOI_FS_EXT_EXT2_H
#define ARKOI_FS_EXT_EXT2_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ext_status {
	EXT_STATUS_OK = 0,					/**< Operation completed successfully. */
	EXT_STATUS_INVALID_ARGUMENT = 1,	/**< Invalid argument provided to function. */
	EXT_STATUS_IO_ERROR = 2,			/**< I/O error occurred during device read/write. */
	EXT_STATUS_BAD_MAGIC = 3,			/**< Filesystem magic number did not match expected value. */
	EXT_STATUS_UNSUPPORTED = 4,			/**< Filesystem features or parameters are not supported. */
	EXT_STATUS_OUT_OF_RANGE = 5			/**< Requested operation is out of valid range (e.g., block or inode number). */
} ext_status;

typedef struct ext_device {
	/* Context pointer passed to device read function, can be used to store state or references. */
	void* context;

	/* Function pointer for reading data from the device. Should return 0 on success, non-zero on failure. */
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

typedef struct ext_block_group_descriptor {
	uint32_t bg_block_bitmap;       /**< Block number of block bitmap. */
	uint32_t bg_inode_bitmap;       /**< Block number of inode bitmap. */
	uint32_t bg_inode_table;       	/**< Starting block number of inode table. */
	uint16_t bg_free_blocks_count; 	/**< Number of free blocks in the group. */
	uint16_t bg_free_inodes_count; 	/**< Number of free inodes in the group. */
	uint16_t bg_used_dirs_count;   	/**< Number of directories in the group. */
	uint16_t bg_pad;               	/**< Padding to align to 32 bytes. */
	uint8_t bg_reserved[12];       	/**< Reserved for future use. */
} ext_block_group_descriptor;

typedef struct ext_inode {
	uint16_t i_mode;     	/**< File mode (type and permissions). */
	uint16_t i_uid;      	/**< Owner UID. */
	uint32_t i_size;       	/**< Size in bytes. */
	uint32_t i_atime;   	/**< Access time (Unix time). */
	uint32_t i_ctime;    	/**< Creation time (Unix time). */
	uint32_t i_mtime;    	/**< Modification time (Unix time). */
	uint32_t i_dtime;   	/**< Deletion time (Unix time). */
	uint16_t i_gid;       	/**< Group ID. */
	uint16_t i_links_count;	/**< Links count. */
	uint32_t i_blocks;    	/**< Blocks count (512-byte units). */
	uint32_t i_flags;     	/**< File flags. */
	uint32_t i_osd1;      	/**< OS-dependent value 1. */
	uint32_t i_block[15];  	/**< Pointers to blocks. */
	uint32_t i_generation;	/**< File version (for NFS). */
	uint32_t i_file_acl;   	/**< File ACL. */
	uint32_t i_dir_acl;  	/**< Directory ACL. */
	uint32_t i_faddr;    	/**< Fragment address. */
	uint8_t i_osd2[12];   	/**< OS-dependent value 2. */
} ext_inode;

typedef struct ext_directory_entry {
	uint32_t inode;    	/**< Inode number. */
	uint16_t rec_len;  	/**< Directory entry length. */
	uint8_t name_len;  	/**< Name length. */
	uint8_t file_type; 	/**< File type (0=unknown, 1=regular, 2=dir, etc.). */
	char name[];    	/**< File name (not null-terminated). */
} ext_directory_entry;

typedef struct ext_filesystem {
	ext_device device;					/**< Device interface for reading filesystem data. */
	ext_superblock superblock;			/**< Cached superblock data for quick access. */
	uint32_t block_size;				/**< Block size in bytes, calculated from superblock. */
	uint32_t inode_size;				/**< Inode size in bytes, from superblock. */
	uint32_t block_group_table_count;	/**< Number of block groups in the filesystem. */	
	uint64_t block_group_table_offset;	/**< Byte offset to the block group descriptor table on the device. */
} ext_filesystem;

/**
 * @brief Mounts an EXT2 filesystem on the specified device.
 *
 * Initializes and mounts an EXT2 filesystem from the provided device,
 * populating the filesystem structure with metadata and preparing it for operations.
 *
 * @param fs Pointer to the `ext_filesystem` structure to be initialized with
 *           the mounted filesystem information.
 * @param device The `ext_device` containing the EXT2 filesystem to mount.
 *
 * @return `ext_status` Status code indicating success or failure of the mount operation.
 *         - Success if the filesystem was mounted successfully.
 *         - Error code if the operation failed (e.g., I/O error, invalid filesystem).
 *
 * @note The filesystem structure must be allocated before calling this function.
 * @note The device must remain valid and accessible for the lifetime of the mount.
 */
ext_status ext2_mount(ext_filesystem* fs, ext_device device);

/**
 * @brief Reads the superblock from an EXT2 filesystem.
 * 
 * Reads the superblock structure from the specified EXT2 filesystem and populates
 * the provided superblock structure with the filesystem metadata.
 * 
 * @param fs Pointer to the `ext_filesystem` structure representing the EXT2 filesystem.
 * @param superblock Pointer to an `ext_superblock` structure where the read superblock
 *                   data will be stored.
 * 
 * @return `ext_status` Status code indicating the result of the operation.
 *         - Success if the superblock was read successfully.
 *         - Error code if the operation failed (e.g., I/O error, invalid filesystem).
 * 
 * @see ext_filesystem, ext_superblock, ext_status
 */
ext_status ext2_read_superblock(const ext_filesystem* fs, ext_superblock* superblock);

/**
 * @brief Reads a block group descriptor from an EXT2 filesystem.
 * 
 * Reads the block group descriptor for the specified block group index from the EXT2 filesystem
 * and populates the provided block group descriptor structure with the relevant metadata.
 * 
 * @param fs Pointer to the `ext_filesystem` structure representing the EXT2 filesystem.
 * @param group_index The index of the block group to read the descriptor for (0-based).
 * @param block_group_descriptor Pointer to an `ext_block_group_descriptor` structure where the read descriptor
 *                               data will be stored.
 * 
 * @return `ext_status` Status code indicating the result of the operation.
 *         - Success if the block group descriptor was read successfully.
 *         - Error code if the operation failed (e.g., I/O error, invalid group index).
 * 
 * @see ext_filesystem, ext_block_group_descriptor, ext_status
 */
ext_status ext2_read_block_group_descriptor(const ext_filesystem* fs, uint32_t group_index, ext_block_group_descriptor* block_group_descriptor);

/**
 * @brief Reads an inode from an EXT2 filesystem.
 * 
 * Reads the inode structure for the specified inode number from the EXT2 filesystem
 * and populates the provided inode structure with the relevant metadata.
 * 
 * @param fs Pointer to the `ext_filesystem` structure representing the EXT2 filesystem.
 * @param number The number of the inode to read (1-based).
 * @param inode Pointer to an `ext_inode` structure where the read inode data will be stored.
 * 
 * @return `ext_status` Status code indicating the result of the operation.
 *         - Success if the inode was read successfully.
 *         - Error code if the operation failed (e.g., I/O error, invalid inode number).
 * 
 * @see ext_filesystem, ext_inode, ext_status
 */
ext_status ext2_read_inode(const ext_filesystem* fs, uint32_t number, ext_inode* inode);

#ifdef __cplusplus
}
#endif

#endif
