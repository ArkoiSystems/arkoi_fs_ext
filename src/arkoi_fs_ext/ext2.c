#include "arkoi_fs_ext/cursor.h"
#include "arkoi_fs_ext/ext2.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

static ext_status decode_block_group_descriptor(const uint8_t* data, ext_block_group_descriptor* descriptor) {
    if (data == NULL || descriptor == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    cursor cursor = { data };

    cursor_read_u32(&cursor, &descriptor->bg_block_bitmap);
    cursor_read_u32(&cursor, &descriptor->bg_inode_bitmap);
    cursor_read_u32(&cursor, &descriptor->bg_inode_table);
    cursor_read_u16(&cursor, &descriptor->bg_free_blocks_count);
    cursor_read_u16(&cursor, &descriptor->bg_free_inodes_count);
    cursor_read_u16(&cursor, &descriptor->bg_used_dirs_count);
    cursor_read_u16(&cursor, &descriptor->bg_pad);
    cursor_read_bytes(&cursor, descriptor->bg_reserved, NELEMS(descriptor->bg_reserved));

    return EXT_STATUS_OK;
}

ext_status ext2_read_block_group_descriptor(const ext_filesystem* fs, uint32_t index, ext_block_group_descriptor* descriptor) {
    if (fs == NULL || descriptor == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    if (index >= fs->block_group_table_count) {
        return EXT_STATUS_OUT_OF_RANGE;
    }

    const uint64_t offset = fs->block_group_table_offset + index * sizeof(ext_block_group_descriptor);
    uint8_t data[sizeof(ext_block_group_descriptor)];

    const ext_status status = device_read(fs, offset, data, sizeof(data));
    if (status != EXT_STATUS_OK) {
        return status;
    }

    return decode_block_group_descriptor(data, descriptor);
}

static ext_status decode_inode(const uint8_t* data, ext_inode* inode) {
    if (data == NULL || inode == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    cursor cursor = { data };

    cursor_read_u16(&cursor, &inode->i_mode);
    cursor_read_u16(&cursor, &inode->i_uid);
    cursor_read_u32(&cursor, &inode->i_size);
    cursor_read_u32(&cursor, &inode->i_atime);
    cursor_read_u32(&cursor, &inode->i_ctime);
    cursor_read_u32(&cursor, &inode->i_mtime);
    cursor_read_u32(&cursor, &inode->i_dtime);
    cursor_read_u16(&cursor, &inode->i_gid);
    cursor_read_u16(&cursor, &inode->i_links_count);
    cursor_read_u32(&cursor, &inode->i_blocks);
    cursor_read_u32(&cursor, &inode->i_flags);
    cursor_read_u32(&cursor, &inode->i_osd1);
    for (uint32_t index = 0U; index < NELEMS(inode->i_block); ++index) {
        cursor_read_u32(&cursor, &inode->i_block[index]);
    }
    cursor_read_u32(&cursor, &inode->i_generation);
    cursor_read_u32(&cursor, &inode->i_file_acl);
    cursor_read_u32(&cursor, &inode->i_dir_acl);
    cursor_read_u32(&cursor, &inode->i_faddr);
    cursor_read_bytes(&cursor, inode->i_osd2, NELEMS(inode->i_osd2));

    return EXT_STATUS_OK;
}

ext_status ext2_read_inode(const ext_filesystem* fs, uint32_t number, ext_inode* inode) {
    if (fs == NULL || inode == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    } 

    if (number == 0 || number > fs->superblock.s_inodes_count) {
        return EXT_STATUS_OUT_OF_RANGE;
    }

    const uint32_t group_index = (number - 1) / fs->superblock.s_inodes_per_group;
    const uint32_t table_index = (number - 1) % fs->superblock.s_inodes_per_group;

    ext_block_group_descriptor descriptor;

    const ext_status group_status = ext2_read_block_group_descriptor(fs, group_index, &descriptor);
    if (group_status != EXT_STATUS_OK) {
        return group_status;
    }

    const uint64_t table_offset = descriptor.bg_inode_table * (uint64_t)(fs->block_size);
    const uint64_t node_offset = table_offset + (table_index * fs->inode_size);
    uint8_t data[fs->inode_size];

    const ext_status device_status = device_read(fs, node_offset, data, sizeof(data));
    if (device_status != EXT_STATUS_OK) {
        return device_status;
    }

    return decode_inode(data, inode);
}

static ext_status read_indirect_block(const ext_filesystem* fs, const uint32_t indirect_block, const uint32_t block_index, uint32_t* block_number) {
    if(fs == NULL || block_number == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    const uint32_t pointers_per_block = fs->block_size / sizeof(uint32_t);
    if(block_index >= pointers_per_block) {
        return EXT_STATUS_OUT_OF_RANGE;
    }

    uint8_t data[fs->block_size];
    const uint64_t block_offset = indirect_block * (uint64_t)(fs->block_size);

    const ext_status device_status = device_read(fs, block_offset, data, sizeof(data));
    if (device_status != EXT_STATUS_OK) {
        return device_status;
    }

    cursor cursor = { data + (block_index * sizeof(uint32_t)) };
    cursor_read_u32(&cursor, block_number);

    return EXT_STATUS_OK;

}

static ext_status resolve_indirect_recursive(const ext_filesystem* fs, const uint32_t indirect_block, const uint32_t level, const uint32_t block_index, uint32_t* block_number) {
    if (fs == NULL || block_number == NULL || level == 0) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    if(level == 1) {
        return read_indirect_block(fs, indirect_block, block_index, block_number);
    }

    const uint32_t pointers_per_block = fs->block_size / sizeof(uint32_t);
    
    const uint32_t current_index = block_index / pointers_per_block;
    const uint32_t next_index = block_index % pointers_per_block;
    
    uint32_t next_block;
    
    ext_status status = read_indirect_block(fs, indirect_block, current_index, &next_block);
    if (status != EXT_STATUS_OK) {
        return status;
    }
    
    if (next_block == 0) {
        *block_number = 0;
        return EXT_STATUS_OK;
    }

    return resolve_indirect_recursive(fs, next_block, level - 1, next_index, block_number);
}

ext_status ext2_resolve_block(const ext_filesystem* fs, const ext_inode* inode, uint32_t block_index, uint32_t* block_number) {
    if (fs == NULL || inode == NULL || block_number == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    const uint32_t direct_range = 12U;
    if(block_index < direct_range) {
        *block_number = inode->i_block[block_index];
        return EXT_STATUS_OK;
    }

    const uint32_t pointers_per_block = fs->block_size / sizeof(uint32_t);

    const uint32_t single_range = pointers_per_block;
    if(block_index < direct_range + single_range) {
        const uint32_t single_index = block_index - direct_range;
        return resolve_indirect_recursive(fs, inode->i_block[12], 1, single_index, block_number);
    }

    const uint32_t double_range = pointers_per_block * pointers_per_block;
    if(block_index < direct_range + single_range + double_range) {
        const uint32_t double_index = block_index - direct_range - single_range;
        return resolve_indirect_recursive(fs, inode->i_block[13], 2, double_index, block_number);
    }

    const uint32_t triple_range = pointers_per_block * pointers_per_block * pointers_per_block;
    if(block_index < direct_range + single_range + double_range + triple_range) {
        const uint32_t triple_index = block_index - direct_range - single_range - double_range;
        return resolve_indirect_recursive(fs, inode->i_block[14], 3, triple_index, block_number);
    }

    return EXT_STATUS_OUT_OF_RANGE;
}

ext_status ext2_lookup_name(const ext_filesystem* fs, const ext_inode* dir, const char* name, ext_inode* found) {
    if (fs == NULL || dir == NULL || name == NULL || found == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    uint8_t data[fs->block_size];

    for(size_t index = 0; index < NELEMS(dir->i_block); ++index) {
        // TODO: Handle indirect blocks when index >= 12U
        if(index >= 12U) {
            continue;
        }

        const uint32_t block_number = dir->i_block[index];
        if (block_number == 0) {
            continue;
        }

        const uint64_t block_offset = block_number * (uint64_t)(fs->block_size);

        const ext_status device_status = device_read(fs, block_offset, data, sizeof(data));
        if (device_status != EXT_STATUS_OK) {
            return device_status;
        }

        size_t offset = 0;
        while (offset < fs->block_size) {
            ext_directory_entry* entry = (ext_directory_entry*)(data + offset);
            if (entry->rec_len == 0) {
                break;
            }

            if (entry->inode != 0) {
                if (entry->name_len == strlen(name) && strncmp(entry->name, name, entry->name_len) == 0) {
                    return ext2_read_inode(fs, entry->inode, found);
                }
            }

            offset += entry->rec_len;
        }
    }

    return EXT_NOT_FOUND;
}

ext_status ext2_lookup_path(const ext_filesystem* fs, const char* path, ext_inode* found) {
    if (fs == NULL || path == NULL || found == NULL || path[0] != '/') {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    ext_inode current_inode;

    const ext_status root_status = ext2_read_inode(fs, 2U, &current_inode);
    if (root_status != EXT_STATUS_OK) {
        return root_status;
    }

    while(*path == '/') {
        ++path;
    }

    while(*path != '\0') {
        ext_inode next_inode;
        
        size_t name_length = 0;
        char name[256];

        while(*path != '/' && *path != '\0') {
            if(name_length > 255) {
                return EXT_STATUS_OUT_OF_RANGE;
            }

            name[name_length++] = *path++;
        }

        name[name_length++] = '\0';

        while(*path == '/') {
            ++path;
        }

        const ext_status next_status = ext2_lookup_name(fs, &current_inode, name, &next_inode);
        if(next_status != EXT_STATUS_OK) {
            return next_status;
        }

        current_inode = next_inode;
    }

    *found = current_inode;

    return EXT_STATUS_OK;
}

ext_status ext2_read_file(const ext_filesystem* fs, const ext_inode* file, uint64_t offset, void* buffer, size_t size) {
    if (fs == NULL || file == NULL || buffer == NULL) {
        return EXT_STATUS_INVALID_ARGUMENT;
    }

    if (offset >= file->i_size) {
        return EXT_STATUS_OUT_OF_RANGE;
    }

    size_t current = offset;
    size_t bytes_read = 0;

    while (bytes_read < size && current < file->i_size) {
        const uint32_t block_index = current / fs->block_size;
        const uint32_t block_offset = current % fs->block_size;

        uint32_t block_number;

        const ext_status resolve_status = ext2_resolve_block(fs, file, block_index, &block_number);
        if (resolve_status != EXT_STATUS_OK) {
            return resolve_status;
        }

        if (block_number == 0) {
            break;
        }

        const uint64_t block_address = block_number * (uint64_t)(fs->block_size);
        const uint64_t read_offset = block_address + block_offset;

        const size_t remaining_in_block = fs->block_size - block_offset;
        const size_t remaining_in_file = file->i_size - current;
        const size_t remaining_requested = size - bytes_read;

        const size_t min_read_size = remaining_in_block < remaining_in_file ? remaining_in_block : remaining_in_file;
        const size_t read_size = min_read_size < remaining_requested ? min_read_size : remaining_requested;

        uint8_t* read_buffer = (uint8_t*)buffer + bytes_read;

        const ext_status device_status = device_read(fs, read_offset, read_buffer, read_size);
        if (device_status != EXT_STATUS_OK) {
            return device_status;
        }

        bytes_read += read_size;
        current += read_size;
    }

    return EXT_STATUS_OK;
}