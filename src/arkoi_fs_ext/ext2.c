#include "arkoi_fs_ext/cursor.h"
#include "arkoi_fs_ext/ext2.h"

#include <stddef.h>
#include <stdint.h>

#define NELEMS(array) (sizeof(array) / sizeof((array)[0]))

static const uint64_t EXT2_SUPERBLOCK_OFFSET = 1024U;
static const uint16_t EXT2_MAGIC = 0xEF53U;

static ext_status device_read(const ext_filesystem* fs, const uint64_t offset, void* data, const size_t bytes) {
    if (fs == NULL || data == NULL || fs->device.read == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    const int status = fs->device.read(fs->device.context, offset, data, bytes);
    if (status != 0) {
        return EXT_STATUS_IO_ERROR;
    }

    return EXT_STATUS_OK;
}

ext_status ext2_mount(ext_filesystem* fs, const ext_device device) {
    if (fs == NULL || device.read == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    fs->device = device;

    const ext_status status = ext2_read_superblock(fs, &fs->superblock);
    if (status != EXT_STATUS_OK) {
        return status;
    }

    fs->block_size = (1024U << fs->superblock.s_log_block_size);
    if (fs->block_size < 1024U || fs->block_size > 65536U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    fs->inode_size = fs->superblock.s_inode_size;
    if (fs->inode_size < 128U || fs->inode_size > fs->block_size || (fs->inode_size & (fs->inode_size - 1U)) != 0U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    if (fs->superblock.s_blocks_per_group == 0U || fs->superblock.s_inodes_per_group == 0U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    fs->block_group_table_count = (fs->superblock.s_blocks_count + fs->superblock.s_blocks_per_group - 1U) / fs->superblock.s_blocks_per_group;
    fs->block_group_table_offset = (fs->block_size == 1024U) ? 2048U : fs->block_size;

    

    return EXT_STATUS_OK;
}


static ext_status decode_superblock(const uint8_t* data, ext_superblock* superblock) {
    if (data == NULL || superblock == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    cursor cursor = { data };

    cursor_read_u32(&cursor, &superblock->s_inodes_count);
    cursor_read_u32(&cursor, &superblock->s_blocks_count);
    cursor_read_u32(&cursor, &superblock->s_r_blocks_count);
    cursor_read_u32(&cursor, &superblock->s_free_blocks_count);
    cursor_read_u32(&cursor, &superblock->s_free_inodes_count);
    cursor_read_u32(&cursor, &superblock->s_first_data_block);
    cursor_read_u32(&cursor, &superblock->s_log_block_size);
    cursor_read_u32(&cursor, &superblock->s_log_frag_size);
    cursor_read_u32(&cursor, &superblock->s_blocks_per_group);
    cursor_read_u32(&cursor, &superblock->s_frags_per_group);
    cursor_read_u32(&cursor, &superblock->s_inodes_per_group);
    cursor_read_u32(&cursor, &superblock->s_mtime);
    cursor_read_u32(&cursor, &superblock->s_wtime);
    cursor_read_u16(&cursor, &superblock->s_mnt_count);
    cursor_read_u16(&cursor, &superblock->s_max_mnt_count);
    cursor_read_u16(&cursor, &superblock->s_magic);
    cursor_read_u16(&cursor, &superblock->s_state);
    cursor_read_u16(&cursor, &superblock->s_errors);
    cursor_read_u16(&cursor, &superblock->s_minor_rev_level);
    cursor_read_u32(&cursor, &superblock->s_lastcheck);
    cursor_read_u32(&cursor, &superblock->s_checkinterval);
    cursor_read_u32(&cursor, &superblock->s_creator_os);
    cursor_read_u32(&cursor, &superblock->s_rev_level);
    cursor_read_u16(&cursor, &superblock->s_def_resuid);
    cursor_read_u16(&cursor, &superblock->s_def_resgid);
    cursor_read_u32(&cursor, &superblock->s_first_ino);
    cursor_read_u16(&cursor, &superblock->s_inode_size);
    cursor_read_u16(&cursor, &superblock->s_block_group_nr);
    cursor_read_u32(&cursor, &superblock->s_feature_compat);
    cursor_read_u32(&cursor, &superblock->s_feature_incompat);
    cursor_read_u32(&cursor, &superblock->s_feature_ro_compat);
    cursor_read_bytes(&cursor, superblock->s_uuid, NELEMS(superblock->s_uuid));
    cursor_read_chars(&cursor, superblock->s_volume_name, NELEMS(superblock->s_volume_name));
    cursor_read_chars(&cursor, superblock->s_last_mounted, NELEMS(superblock->s_last_mounted));
    cursor_read_u32(&cursor, &superblock->s_algo_bitmap);
    cursor_read_u8(&cursor, &superblock->s_prealloc_blocks);
    cursor_read_u8(&cursor, &superblock->s_prealloc_dir_blocks);
    cursor_read_bytes(&cursor, superblock->s_padding_1, NELEMS(superblock->s_padding_1));
    cursor_read_bytes(&cursor, superblock->s_journal_uuid, NELEMS(superblock->s_journal_uuid));
    cursor_read_u32(&cursor, &superblock->s_journal_inum);
    cursor_read_u32(&cursor, &superblock->s_journal_dev);
    cursor_read_u32(&cursor, &superblock->s_last_orphan);
    for (uint32_t index = 0U; index < NELEMS(superblock->s_hash_seed); ++index) {
        cursor_read_u32(&cursor, &superblock->s_hash_seed[index]);
    }
    cursor_read_u8(&cursor, &superblock->s_def_hash_version);
    cursor_read_bytes(&cursor, superblock->s_padding_2, NELEMS(superblock->s_padding_2));
    cursor_read_u32(&cursor, &superblock->s_default_mount_opts);
    cursor_read_u32(&cursor, &superblock->s_first_meta_bg);
    cursor_read_bytes(&cursor, superblock->s_padding_3, NELEMS(superblock->s_padding_3));

    if (superblock->s_magic != EXT2_MAGIC) {
        return EXT_STATUS_BAD_MAGIC;
    }

    return EXT_STATUS_OK;
}

ext_status ext2_read_superblock(const ext_filesystem* fs, ext_superblock* superblock) {
    if (fs == NULL || superblock == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    uint8_t data[1024];

    const ext_status status = device_read(fs, EXT2_SUPERBLOCK_OFFSET, data, sizeof(data));
    if (status != EXT_STATUS_OK) {
        return status;
    }

    return decode_superblock(data, superblock);
}

static ext_status decode_block_group_descriptor(const uint8_t* data, ext_block_group_descriptor* block_group_descriptor) {
    if (data == NULL || block_group_descriptor == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    cursor cursor = { data };

    cursor_read_u32(&cursor, &block_group_descriptor->bg_block_bitmap);
    cursor_read_u32(&cursor, &block_group_descriptor->bg_inode_bitmap);
    cursor_read_u32(&cursor, &block_group_descriptor->bg_inode_table);
    cursor_read_u16(&cursor, &block_group_descriptor->bg_free_blocks_count);
    cursor_read_u16(&cursor, &block_group_descriptor->bg_free_inodes_count);
    cursor_read_u16(&cursor, &block_group_descriptor->bg_used_dirs_count);
    cursor_read_u16(&cursor, &block_group_descriptor->bg_pad);
    cursor_read_bytes(&cursor, block_group_descriptor->bg_reserved, NELEMS(block_group_descriptor->bg_reserved));

    return EXT_STATUS_OK;
}

ext_status ext2_read_block_group_descriptor(const ext_filesystem* fs, uint32_t group_index, ext_block_group_descriptor* block_group_descriptor) {
    if (fs == NULL || block_group_descriptor == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    if (group_index >= fs->block_group_table_count) {
        return EXT_STATUS_OUT_OF_RANGE;
    }

    const uint64_t offset = fs->block_group_table_offset + group_index * sizeof(ext_block_group_descriptor);
    uint8_t data[sizeof(ext_block_group_descriptor)];

    const ext_status status = device_read(fs, offset, data, sizeof(data));
    if (status != EXT_STATUS_OK) {
        return status;
    }

    return decode_block_group_descriptor(data, block_group_descriptor);
}