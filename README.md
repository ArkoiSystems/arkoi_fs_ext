<h1 align="center" id="title">Arkoi EXT2 FileSystem</h1>

<p align="center" id="description">
EXT2 filesystem reader library for ArkoiOS. Designed to be freestanding-safe so it can be linked directly into the kernel without a hosted standard library.
</p>

---

## Features

- EXT2 superblock parsing and validation
- Block group descriptor table parsing
- Inode table lookup and inode parsing
- Pluggable block device interface — bring your own I/O backend

## Building

```sh
cmake -S . -B build
cmake --build build
```

To also build and run the tests:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## API

Include `arkoi_fs_ext/ext2.hpp` and implement an `ext_device` with a `read` callback that suits your storage layer:

```c
ext_device device = {
    .context = &my_storage,
    .read    = my_read_fn,
};

ext_filesystem fs;
ext2_mount(&fs, device);

ext_inode root;
ext2_read_inode(&fs, 2, &root);
```

Full API surface:

| Function                     | Description                                             |
| ---------------------------- | ------------------------------------------------------- |
| `ext2_mount`                 | Validate and mount a filesystem, parsing the superblock |
| `ext2_read_superblock`       | Re-read the superblock into an `ext_superblock`         |
| `ext2_read_group_descriptor` | Read a block group descriptor by index                  |
| `ext2_read_inode`            | Read an inode by inode number                           |

## Project Layout

```
arkoi_fs_ext/
├── include/arkoi_fs_ext/
│   └── ext2.hpp          # Public API header
├── src/
│   └── arkoi_fs_ext/
│       └── ext2.c        # EXT2 parser implementation
├── tests/
│   ├── fixtures/         # Real EXT2 disk images used by tests
│   └── arkoi_fs_ext/
│       └── ext2.cpp      # GoogleTest test suite
└── cmake/                # CMake package config helpers
```
