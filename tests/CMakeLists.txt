include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

set(INSTALL_GTEST OFF CACHE BOOL "Disable GoogleTest install")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(${PROJECT_NAME}_tests
        main.cpp
        arkoi_fs_ext/ext2.cpp)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME} gtest_main)

target_compile_definitions(${PROJECT_NAME}_tests PRIVATE
        ARKOI_TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

gtest_discover_tests(${PROJECT_NAME}_tests)