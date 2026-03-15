#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "arkoi_fs_ext/ext2.hpp"

#ifndef ARKOI_TEST_FIXTURES_DIR
#error "ARKOI_TEST_FIXTURES_DIR must be defined via CMake"
#endif

struct FileDevice {
    std::vector<uint8_t> data;
    bool fail_reads = false;
};

FileDevice create_device(std::string_view fixture) {
    const std::filesystem::path path = std::filesystem::path(ARKOI_TEST_FIXTURES_DIR) / fixture;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    EXPECT_TRUE(file.is_open()) << "Could not open fixture: " << path;

    FileDevice device{};
    if (file.is_open()) {
        const auto size = static_cast<std::size_t>(file.tellg());
        device.data.resize(size);

        file.seekg(0);
        file.read(reinterpret_cast<char*>(device.data.data()), static_cast<std::streamsize>(size));
    }

    return device;
}

int file_device_read(void* context, uint64_t offset, void* out_data, size_t bytes) {
    auto* device = static_cast<FileDevice*>(context);
    if (device->fail_reads) {
        return -1;
    }

    if (offset > device->data.size() || bytes > device->data.size() - offset) {
        return -1;
    }

    std::copy_n(device->data.data() + offset, bytes, static_cast<uint8_t*>(out_data));
    return 0;
}

ext_device make_device(FileDevice* dev) {
    return ext_device{ .context = dev, .read = file_device_read };
}

TEST(Ext2ApiTest, MountParsesSuperblockAndGeometry) {
    FileDevice device = create_device("valid.img");
    ext_filesystem ext{};

    const ext_status mount_status = ext2_mount(&ext, make_device(&device));
    EXPECT_EQ(mount_status, EXT_STATUS_OK);

    EXPECT_EQ(ext.block_size, 1024U);
    EXPECT_EQ(ext.inode_size, 128U);
    EXPECT_EQ(ext.group_count, 1U);
    EXPECT_EQ(ext.superblock.magic, 0xEF53U);
}

TEST(Ext2ApiTest, ReadsGroupDescriptor) {
    FileDevice device = create_device("valid.img");
    ext_filesystem ext{};
    
    const ext_status mount_status = ext2_mount(&ext, make_device(&device));
    EXPECT_EQ(mount_status, EXT_STATUS_OK);

    ext_group_descriptor group_desc{};
    const ext_status status = ext2_read_group_descriptor(&ext, 0U, &group_desc);

    EXPECT_EQ(status, EXT_STATUS_OK);
    EXPECT_EQ(group_desc.inode_table, 5U);
    EXPECT_EQ(group_desc.used_dirs_count, 2U);
}

TEST(Ext2ApiTest, ReadsRootInode) {
    FileDevice device = create_device("valid.img");
    ext_filesystem ext{};

    const ext_status mount_status = ext2_mount(&ext, make_device(&device));
    EXPECT_EQ(mount_status, EXT_STATUS_OK);

    ext_inode inode{};
    const ext_status status = ext2_read_inode(&ext, 2U, &inode);

    EXPECT_EQ(status, EXT_STATUS_OK);
    EXPECT_EQ(inode.mode, 0x41EDU);
    EXPECT_EQ(inode.size, 1024U);
    EXPECT_EQ(inode.block[0], 9U);
}

TEST(Ext2ApiTest, RejectsBadMagic) {
    FileDevice device = create_device("bad_magic.img");
    ext_filesystem ext{};

    const ext_status status = ext2_mount(&ext, make_device(&device));
    EXPECT_EQ(status, EXT_STATUS_BAD_MAGIC);
}

TEST(Ext2ApiTest, PropagatesReadErrors) {
    FileDevice device = create_device("valid.img");
    ext_filesystem ext{};
    
    device.fail_reads = true;

    const ext_status mount_status = ext2_mount(&ext, make_device(&device));
    EXPECT_EQ(mount_status, EXT_STATUS_IO_ERROR);
}

TEST(Ext2ApiTest, RejectsNullFilesystemInMount) {
    FileDevice device = create_device("valid.img");

    const ext_status mount_status = ext2_mount(nullptr, make_device(&device));
    EXPECT_EQ(mount_status, EXT_STATUS_INVALID_ARGUMENT);
}

TEST(Ext2ApiTest, RejectsOutOfRangeGroupDescriptorIndex) {
    FileDevice device = create_device("valid.img");
    ext_filesystem ext{};

    const ext_status mount_status = ext2_mount(&ext, make_device(&device));
    EXPECT_EQ(mount_status, EXT_STATUS_OK);

    ext_group_descriptor group_desc{};
    const ext_status group_status = ext2_read_group_descriptor(&ext, ext.group_count, &group_desc);
    EXPECT_EQ(group_status, EXT_STATUS_OUT_OF_RANGE);
}

TEST(Ext2ApiTest, RejectsZeroInodeNumber) {
    FileDevice device = create_device("valid.img");
    ext_filesystem ext{};

    const ext_status mount_status = ext2_mount(&ext, make_device(&device));
    EXPECT_EQ(mount_status, EXT_STATUS_OK);

    ext_inode inode{};

    const ext_status inode_status = ext2_read_inode(&ext, 0U, &inode);
    EXPECT_EQ(inode_status, EXT_STATUS_INVALID_ARGUMENT);
}
