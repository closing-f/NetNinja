cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
project(test)
# include_directories("include/logger.h")
# set(LIB_SRC
#     logger/logger.cpp
#     utils.cpp)

add_library(servercc SHARED src/logger.cpp src/utils.cpp src/config.cpp src/mutex.h src/thread.h)
# add_subdirectory(logger)

include_directories(/usr/local/include/yaml-cpp)
# test.cpp代表你的测试文件
add_executable(test test_thread.cpp)
add_dependencies(test servercc)
target_link_libraries(test servercc libyaml-cpp.so)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)