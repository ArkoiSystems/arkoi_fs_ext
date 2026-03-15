# cmake-template

![State](https://img.shields.io/github/actions/workflow/status/Excse/cmake-template/cmake-single-platform.yml?style=flat&label=Build%2FTest)
![Last Commit](https://img.shields.io/github/last-commit/Excse/cmake-template?style=flat&label=Last%20Commit)
![Contributors](https://img.shields.io/github/contributors/Excse/cmake-template?style=flat&label=Contributors)
![Visitors](https://api.visitorbadge.io/api/visitors?path=https%3A%2F%2Fgithub.com%2FExcse%2Fcmake-template&label=Repository%20Visits&countColor=%232ccce4&style=flat&labelStyle=none)
![License](https://img.shields.io/github/license/Excse/cmake-template?style=flat&label=License)
![Stars](https://img.shields.io/github/stars/Excse/cmake-template)

A template structure for a modern CMake project, offering a robust starting point for C++ library and app development using CMake 4.2+.

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
  - [1. Clone the template](#1-clone-the-template)
  - [2. Configure the project](#2-configure-the-project)
  - [3. Build the library](#3-build-the-library-and-executable-if-enabled)
  - [4. Run Unit-Tests](#4-run-unit-tests-only-if-enabled)
  - [5. Install the library](#5-install-the-library-only-if-enabled)
  - [6. Uninstall the library](#6-uninstall-the-library-only-if-the-library-was-installed)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Testing](#testing)
- [Documentation](#documentation)
- [Customization](#customization)
- [License](#license)

## Features

- Modern **CMake 4.2+**
- **C++20** enabled by default
- Out-of-the-box **library target** with namespaced alias
- **Header/source separation**: designed for library development
- **Exported CMake package** for easy consumption via `find_package`
- **Testing** via GoogleTest (FetchContent)
- Clean, minimal, and ready to extend

## Getting Started

### 1. Clone the template

```sh
git clone https://github.com/Excse/cmake-template.git
cd cmake-template
```

### 2. Configure the project
There are several options you can enable/disable to your liking (or remove the related code if not needed):
- `BUILD_TESTING` (ON/OFF): Builds unit tests. Turn OFF for faster builds when you only need the library/app.
- `BUILD_EXECUTABLE` (ON/OFF): Builds an example app. Provide a `src/main.cpp` when ON.
- `MAKE_INSTALLABLE` (ON/OFF): Enables `install` rules for packaging/installation.
- `BUILD_SHARED_LIBS` (ON/OFF): Choose shared vs. static library.

```sh
cmake -S . -B build -DBUILD_TESTING=ON         \
                    -DBUILD_EXECUTABLE=ON      \
                    -DMAKE_INSTALLABLE=ON      \
                    -DBUILD_SHARED_LIBS=ON
```

### 3. Build the library (and executable, if enabled)

```sh
cmake --build build
```

### 4. Run Unit-Tests (only if enabled)

```sh
cd build
ctest --output-on-failure
```

### 5. Install the library (only if enabled)

```sh
cmake --install build
```

### 6. Uninstall the library (only if the library was installed)

```sh
# Remove files listed by CMake during install
sudo xargs rm < build/install_manifest.txt
```

## Usage
After installing, you can use this library in another CMake project via:

```cmake
find_package(your_project REQUIRED)

target_link_libraries(your_target PRIVATE your_project::your_project)
```

Notes:
- The exported package name matches the project name in `CMakeLists.txt` (`project(your_project ...)`).
- The namespaced target is `your_project::your_project`.

## Project Structure

```text
cmake-template/
├── CMakeLists.txt
├── include/
│   └── your_project/
│       └── library.hpp # sample public header
├── src/
│   ├── your_project/
│   │   └── library.cpp # library implementation
│   └── main.cpp        # example app entry (when BUILD_EXECUTABLE=ON)
├── tests/
│   └── CMakeLists.txt  # GoogleTest integration (FetchContent)
└── cmake/              # package config template(s)
```

## Testing

- GoogleTest is automatically downloaded using CMake's FetchContent.
- All `.cpp` files in `tests/` are compiled and run as unit tests.
- Run `ctest --output-on-failure` from the build directory.

## Documentation

- A ready-to-use `Doxyfile` is included. Generate docs locally with:
  ```sh
  doxygen Doxyfile
  ```
- A GitHub Actions workflow is provided to publish Doxygen docs to GitHub Pages (see `.github/workflows/doxygen-gh-pages.yml`).

## Customization

- Change `project(your_project ...)` in `CMakeLists.txt` to your project name.
- Add your source/header files under `src/` and `include/`.
- Adjust install rules as needed for your platform or packaging strategy.
- Toggle options in configure step (see Getting Started) or remove unneeded code paths.

## License
This project is a template and does not include a license by default. Add a `LICENSE` file appropriate for your use.
