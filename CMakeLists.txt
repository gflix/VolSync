cmake_minimum_required(VERSION 3.10)

# set project name
project(vol-sync LANGUAGES CXX)

# require compiler features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# require external packages and programs
# find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# import directory names used on GNU systems
include(GNUInstallDirs)

# include build scripts
include(src.cmake)
