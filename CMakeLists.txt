cmake_minimum_required(VERSION 3.16)
set(PROJECT_VER "0.2.1")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tab5gotchi)
# Same FatFs exFAT ABI for BSP and application, without editing the SDK.
set(FAT_CONFIG "${CMAKE_BINARY_DIR}/gotchi_fatfs.h")
set(IDF_PATH_FOR_FATFS "$ENV{IDF_PATH}")
configure_file(tools/fatfs_config.h.in "${FAT_CONFIG}" @ONLY)
idf_build_set_property(COMPILE_OPTIONS "-include${FAT_CONFIG}" APPEND)
