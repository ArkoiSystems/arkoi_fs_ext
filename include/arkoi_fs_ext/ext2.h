#ifndef ARKOI_FS_EXT_EXT2_H
#define ARKOI_FS_EXT_EXT2_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Used for filesystem status flags: s_state */
#define EXT2_VALID_FS (1U)
#define EXT2_ERROR_FS (2U)

/* Used for filesystem error handling behavior: s_errors */
#define EXT2_ERRORS_CONTINUE (1U)
#define EXT2_ERRORS_RO 		 (2U)
#define EXT2_ERRORS_PANIC    (3U)

/* Used for operating system type: s_creator_os */
#define EXT2_OS_LINUX   (0U)
#define EXT2_OS_HURD    (1U)
#define EXT2_OS_MASIX   (2U)
#define EXT2_OS_FREEBSD (3U)
#define EXT2_OS_LITES   (4U)

/* Used for filesystem revision level: s_rev_level */
#define EXT2_GOOD_OLD_REV (0U)
#define EXT2_DYNAMIC_REV  (1U)

/* Used for compatible feature flags: s_feature_compat */
#define EXT2_FEATURE_COMPAT_DIR_PREALLOC  (0x0001U)
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES (0x0002U)
#define EXT2_FEATURE_COMPAT_HAS_JOURNAL   (0x0004U)
#define EXT2_FEATURE_COMPAT_EXT_ATTR      (0x0008U)
#define EXT2_FEATURE_COMPAT_RESIZE_INODES (0x0010U)
#define EXT2_FEATURE_COMPAT_DIR_INDEX     (0x0020U)

/* Used for incompatible feature flags: s_feature_incompat */
#define EXT2_FEATURE_INCOMPAT_COMPRESSION (0x0001U)
#define EXT2_FEATURE_INCOMPAT_FILETYPE    (0x0002U)
#define EXT2_FEATURE_INCOMPAT_RECOVER     (0x0004U)
#define EXT2_FEATURE_INCOMPAT_JOURNAL_DEV (0x0008U)
#define EXT2_FEATURE_INCOMPAT_META_BG     (0x0010U)

/* Used for read-only compatible feature flags: s_feature_ro_compat */
#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER (0x0001U)
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   (0x0002U)
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    (0x0004U)

/* Used for compression algorithm flags: s_algo_bitmap */
#define EXT2_LZV1_ALG   (1U)
#define EXT2_LZRW3A_ALG (2U)
#define EXT2_GZIP_ALG   (3U)
#define EXT2_BZIP2_ALG  (4U)
#define EXT2_LZO_ALG	(5U)

/* Reserved inodes */
#define EXT2_BAD_INO         (1U)
#define EXT2_ROOT_INO        (2U)
#define EXT2_ACL_IDX_INO     (3U)
#define EXT2_ACL_DATA_INO    (4U)
#define EXT2_BOOT_LOADER_INO (5U)
#define EXT2_UNDEL_DIR_INO   (6U)

/* Used to indicate the format of the file: i_mode */
/* --- FILE FORMAT --- */
#define EXT2_S_IFSOCK (0xC000U)
#define EXT2_S_IFLNK  (0xA000U)
#define EXT2_S_IFREG  (0x8000U)
#define EXT2_S_IFBLK  (0x6000U)
#define EXT2_S_IFDIR  (0x4000U)
#define EXT2_S_IFCHR  (0x2000U)
#define EXT2_S_IFIFO  (0x1000U)
/* --- PROCESS EXECUTION USER/GROUP OVERRIDES --- */
#define EXT2_S_ISUID (0x0800U)
#define EXT2_S_ISGID (0x0400U)
#define EXT2_S_ISVTX (0x0200U)
/* --- ACCESS RIGHTS --- */
#define EXT2_S_IRUSR (0x0100U)
#define EXT2_S_IWUSR (0x0080U)
#define EXT2_S_IXUSR (0x0040U)
#define EXT2_S_IRGRP (0x0020U)
#define EXT2_S_IWGRP (0x0010U)
#define EXT2_S_IXGRP (0x0008U)
#define EXT2_S_IROTH (0x0004U)
#define EXT2_S_IWOTH (0x0002U)
#define EXT2_S_IXOTH (0x0001U)

/* Used for file attributes: i_flags */
#define EXT2_SECRM_FL     (0x00000001U)
#define EXT2_UNRM_FL      (0x00000002U)
#define EXT2_COMPR_FL     (0x00000004U)
#define EXT2_SYNC_FL      (0x00000008U)
#define EXT2_IMMUTABLE_FL (0x00000010U)
#define EXT2_APPEND_FL    (0x00000020U)
#define EXT2_NODUMP_FL    (0x00000040U)
#define EXT2_NOATIME_FL   (0x00000080U)
/* --- RESERVED FOR COMPRESSION USAGE --- */
#define EXT2_DIRTY_FL    (0x00000100U)
#define EXT2_COMPRBLK_FL (0x00000200U)
#define EXT2_NOCOMPR_FL  (0x00000400U)
#define EXT2_ECOMPR_FL   (0x00000800U)
/* --- END OF COMPRESSION FLAGS --- */
#define EXT2_BTREE_FL        (0x00001000U)
#define EXT2_INDEX_FL        (0x00002000U)
#define EXT2_IMAGIC_FL       (0x00004000U)
#define EXT2_JOURNAL_DATA_FL (0x00008000U)
#define EXT2_RESERVED_FL     (0x80000000U)

/* Used for file type indicators: file_type */
#define EXT2_FT_UNKNOWN  (0U)
#define EXT2_FT_REG_FILE (1U)
#define EXT2_FT_DIR      (2U)
#define EXT2_FT_CHRDEV   (3U)
#define EXT2_FT_BLKDEV   (4U)
#define EXT2_FT_FIFO     (5U)
#define EXT2_FT_SOCK     (6U)
#define EXT2_FT_SYMLINK  (7U)

typedef enum ext_status {
	EXT_STATUS_OK = 0,					/**< Operation completed successfully. */
	EXT_STATUS_INVALID_ARGUMENT = 1,	/**< Invalid argument provided to function. */
	EXT_STATUS_IO_ERROR = 2,			/**< I/O error occurred during device read/write. */
	EXT_STATUS_BAD_MAGIC = 3,			/**< Filesystem magic number did not match expected value. */
	EXT_STATUS_UNSUPPORTED = 4,			/**< Filesystem features or parameters are not supported. */
	EXT_STATUS_OUT_OF_RANGE = 5,		/**< Requested operation is out of valid range (e.g., block or inode number). */
	EXT_NOT_FOUND = 6,					/**< Requested file or directory was not found. */
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

/**
 * @brief Looks up an inode by name within a directory.
 * 
 * Searches for an inode with the specified name within the given directory
 * and populates the provided inode structure with the found metadata.
 * 
 * @param fs Pointer to the `ext_filesystem` structure representing the EXT2 filesystem.
 * @param dir Pointer to the `ext_inode` structure representing the directory to search.
 * @param name The name of the inode to look up.
 * @param found Pointer to an `ext_inode` structure where the found inode data will be stored.
 * 
 * @return `ext_status` Status code indicating the result of the operation.
 *         - Success if the inode was found and read successfully.
 *         - Error code if the operation failed (e.g., I/O error, invalid directory, or file not found).
 * 
 * @see ext_filesystem, ext_inode, ext_status
 */
ext_status ext2_lookup_name(const ext_filesystem* fs, const ext_inode* dir, const char* name, ext_inode* found);

/**
 * @brief Looks up an inode by path.
 * 
 * Searches for an inode at the specified path within the EXT2 filesystem
 * and populates the provided inode structure with the found metadata.
 * 
 * @param fs Pointer to the `ext_filesystem` structure representing the EXT2 filesystem.
 * @param path The path of the inode to look up.
 * @param found Pointer to an `ext_inode` structure where the found inode data will be stored.
 * 
 * @return `ext_status` Status code indicating the result of the operation.
 *         - Success if the inode was found and read successfully.
 *         - Error code if the operation failed (e.g., I/O error, invalid path, or file not found).
 * 
 * @see ext_filesystem, ext_inode, ext_status
 */
ext_status ext2_lookup_path(const ext_filesystem* fs, const char* path, ext_inode* found);

/**
 * @brief Reads data from a file represented by an inode.
 * 
 * Reads data from the specified file inode at the given offset into the provided buffer.
 * 
 * @param fs Pointer to the `ext_filesystem` structure representing the EXT2 filesystem.
 * @param file Pointer to the `ext_inode` structure representing the file to read from.
 * @param offset The byte offset within the file to start reading from.
 * @param buffer Pointer to a buffer where the read data will be stored.
 * @param size The number of bytes to read into the buffer.
 * 
 * @return `ext_status` Status code indicating the result of the operation.
 *         - Success if the data was read successfully.
 *         - Error code if the operation failed (e.g., I/O error, invalid inode, or read beyond end of file).
 * 
 * @see ext_filesystem, ext_inode, ext_status
 */
ext_status ext2_read_file(const ext_filesystem* fs, const ext_inode* file, uint64_t offset, void* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif
