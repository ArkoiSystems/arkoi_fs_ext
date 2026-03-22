#include "arkoi_fs_ext/cursor.h"
#include "arkoi_fs_ext/ext2.h"

#include <stddef.h>
#include <stdint.h>

static const uint32_t EXT2_GROUP_DESCRIPTOR_SIZE = 32U;
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
    cursor_read_bytes(&cursor, superblock->s_uuid, 16U);
    cursor_read_chars(&cursor, superblock->s_volume_name, 16U);
    cursor_read_chars(&cursor, superblock->s_last_mounted, 64U);
    cursor_read_u32(&cursor, &superblock->s_algo_bitmap);
    cursor_read_u8(&cursor, &superblock->s_prealloc_blocks);
    cursor_read_u8(&cursor, &superblock->s_prealloc_dir_blocks);
    cursor_read_bytes(&cursor, superblock->s_padding_1, 2U);
    cursor_read_bytes(&cursor, superblock->s_journal_uuid, 16U);
    cursor_read_u32(&cursor, &superblock->s_journal_inum);
    cursor_read_u32(&cursor, &superblock->s_journal_dev);
    cursor_read_u32(&cursor, &superblock->s_last_orphan);
    for (uint32_t index = 0U; index < 4U; ++index) {
        cursor_read_u32(&cursor, &superblock->s_hash_seed[index]);
    }
    cursor_read_u8(&cursor, &superblock->s_def_hash_version);
    cursor_read_bytes(&cursor, superblock->s_padding_2, 3U);
    cursor_read_u32(&cursor, &superblock->s_default_mount_opts);
    cursor_read_u32(&cursor, &superblock->s_first_meta_bg);
    cursor_read_bytes(&cursor, superblock->s_padding_3, 760U);

    if (superblock->s_magic != EXT2_MAGIC) {
        return EXT_STATUS_BAD_MAGIC;
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

    fs->block_size = 1024U << fs->superblock.s_log_block_size;
    if (fs->block_size < 1024U || fs->block_size > 65536U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    fs->inode_size = fs->superblock.s_inode_size;
    if (fs->inode_size == 0U) {
        fs->inode_size = 128U;
    }

    if (fs->inode_size < 128U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    if (fs->superblock.s_blocks_per_group == 0U || fs->superblock.s_inodes_per_group == 0U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    fs->group_count = (fs->superblock.s_blocks_count + fs->superblock.s_blocks_per_group - 1U) / fs->superblock.s_blocks_per_group;
    fs->group_descriptor_table_offset = (fs->block_size == 1024U) ? 2048U : fs->block_size;

    return EXT_STATUS_OK;
}

ext_status ext2_read_superblock(const ext_filesystem* fs, ext_superblock* superblock) {
    if (fs == NULL || superblock == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    uint8_t sb_bytes[1024U];
    const ext_status status = device_read(fs, EXT2_SUPERBLOCK_OFFSET, sb_bytes, sizeof(sb_bytes));
    if (status != EXT_STATUS_OK) {
        return status;
    }

    return decode_superblock(sb_bytes, superblock);
}

ext_status ext2_read_group_descriptor(const ext_filesystem* fs, const uint32_t index, ext_group_descriptor* desc) {
    if (fs == NULL || desc == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    if (index >= fs->group_count) {
        return EXT_STATUS_OUT_OF_RANGE;
    }

    const uint64_t offset = fs->group_descriptor_table_offset + ((uint64_t) index * EXT2_GROUP_DESCRIPTOR_SIZE);

    uint8_t data[32U];
    const ext_status status = device_read(fs, offset, data, sizeof(data));
    if (status != EXT_STATUS_OK) {
        return status;
    }

    cursor cur = { data };

    cursor_read_u32(&cur, &desc->block_bitmap);
    cursor_read_u32(&cur, &desc->inode_bitmap);
    cursor_read_u32(&cur, &desc->inode_table);
    cursor_read_u16(&cur, &desc->free_blocks_count);
    cursor_read_u16(&cur, &desc->free_inodes_count);
    cursor_read_u16(&cur, &desc->used_dirs_count);
    cursor_read_u16(&cur, &desc->pad);
    cursor_read_bytes(&cur, (uint8_t*) desc->reserved, sizeof(desc->reserved));

    return EXT_STATUS_OK;
}

ext_status ext2_read_inode(const ext_filesystem* fs, const uint32_t number, ext_inode* inode) {
    if (fs == NULL || inode == NULL || number == 0U) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    const uint32_t inode_index = number - 1U;
    const uint32_t group = inode_index / fs->superblock.s_inodes_per_group;
    const uint32_t group_index = inode_index % fs->superblock.s_inodes_per_group;

    ext_group_descriptor desc;
    ext_status status = ext2_read_group_descriptor(fs, group, &desc);
    if (status != EXT_STATUS_OK) {
        return status;
    }

    uint8_t data[256U];
    if (fs->inode_size > sizeof(data)) {
        return EXT_STATUS_UNSUPPORTED;
    }

    const uint64_t offset = ((uint64_t) desc.inode_table * fs->block_size) + ((uint64_t) group_index * fs->inode_size);
    status = device_read(fs, offset, data, fs->inode_size);
    if (status != EXT_STATUS_OK) {
        return status;
    }

    cursor cur = { data };

    cursor_read_u16(&cur, &inode->mode);
    cursor_read_u16(&cur, &inode->uid);
    cursor_read_u32(&cur, &inode->size);
    cursor_read_u32(&cur, &inode->atime);
    cursor_read_u32(&cur, &inode->ctime);
    cursor_read_u32(&cur, &inode->mtime);
    cursor_read_u32(&cur, &inode->dtime);
    cursor_read_u16(&cur, &inode->gid);
    cursor_read_u16(&cur, &inode->links_count);
    cursor_read_u32(&cur, &inode->blocks);
    cursor_read_u32(&cur, &inode->flags);
    cur.data += 4U;

    for (uint32_t index = 0U; index < 15U; ++index) {
        cursor_read_u32(&cur, &inode->block[index]);
    }

    return EXT_STATUS_OK;
}
