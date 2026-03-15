#include "arkoi_fs_ext/ext2.hpp"

#include <stddef.h>
#include <stdint.h>

static const uint32_t EXT2_GROUP_DESCRIPTOR_SIZE = 32U;
static const uint64_t EXT2_SUPERBLOCK_OFFSET = 1024U;
static const uint16_t EXT2_MAGIC = 0xEF53U;

static uint16_t read_le16(const uint8_t* data) {
    return (uint16_t)(data[0]) | (uint16_t)(data[1] << 8U);
}

static uint32_t read_le32(const uint8_t* data) {
    return (uint32_t)(data[0]) | (uint32_t)(data[1] << 8U) | (uint32_t)(data[2] << 16U) | (uint32_t)(data[3] << 24U);
}

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

    superblock->inodes_count = read_le32(data + 0U);
    superblock->blocks_count = read_le32(data + 4U);
    superblock->reserved_blocks_count = read_le32(data + 8U);
    superblock->free_blocks_count = read_le32(data + 12U);
    superblock->free_inodes_count = read_le32(data + 16U);
    superblock->first_data_block = read_le32(data + 20U);
    superblock->log_block_size = read_le32(data + 24U);
    superblock->blocks_per_group = read_le32(data + 32U);
    superblock->inodes_per_group = read_le32(data + 40U);
    superblock->magic = read_le16(data + 56U);
    superblock->revision_level = read_le32(data + 76U);
    superblock->inode_size = read_le16(data + 88U);
    superblock->feature_compat = read_le32(data + 92U);
    superblock->feature_incompat = read_le32(data + 96U);
    superblock->feature_ro_compat = read_le32(data + 100U);

    if (superblock->magic != EXT2_MAGIC) {
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

    fs->block_size = 1024U << fs->superblock.log_block_size;
    if (fs->block_size < 1024U || fs->block_size > 65536U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    fs->inode_size = fs->superblock.inode_size;
    if (fs->inode_size == 0U) {
        fs->inode_size = 128U;
    }

    if (fs->inode_size < 128U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    if (fs->superblock.blocks_per_group == 0U || fs->superblock.inodes_per_group == 0U) {
        return EXT_STATUS_UNSUPPORTED;
    }

    fs->group_count = (fs->superblock.blocks_count + fs->superblock.blocks_per_group - 1U) / fs->superblock.blocks_per_group;
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

    desc->block_bitmap = read_le32(data + 0U);
    desc->inode_bitmap = read_le32(data + 4U);
    desc->inode_table = read_le32(data + 8U);
    desc->free_blocks_count = read_le16(data + 12U);
    desc->free_inodes_count = read_le16(data + 14U);
    desc->used_dirs_count = read_le16(data + 16U);

    return EXT_STATUS_OK;
}

ext_status ext2_read_inode(const ext_filesystem* fs, const uint32_t number, ext_inode* inode) {
    if (fs == NULL || inode == NULL || number == 0U) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    const uint32_t inode_index = number - 1U;
    const uint32_t group = inode_index / fs->superblock.inodes_per_group;
    const uint32_t group_index = inode_index % fs->superblock.inodes_per_group;

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

    inode->mode = read_le16(data + 0U);
    inode->uid = read_le16(data + 2U);
    inode->size = read_le32(data + 4U);
    inode->atime = read_le32(data + 8U);
    inode->ctime = read_le32(data + 12U);
    inode->mtime = read_le32(data + 16U);
    inode->dtime = read_le32(data + 20U);
    inode->gid = read_le16(data + 24U);
    inode->links_count = read_le16(data + 26U);
    inode->blocks = read_le32(data + 28U);
    inode->flags = read_le32(data + 32U);

    for (uint32_t index = 0U; index < 15U; ++index) {
        inode->block[index] = read_le32(data + 40U + (index * 4U));
    }

    return EXT_STATUS_OK;
}
