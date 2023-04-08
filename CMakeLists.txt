cmake_minimum_required(VERSION 2.8)

project(logtest)
# include_directories("include/logger.h")
# set(LIB_SRC
#     logger/logger.cpp
#     utils.cpp)

add_library(servercc_lib SHARED logger/logger.cpp utils.cpp)
# add_subdirectory(logger)

add_executable(logtest test.cpp)
add_dependencies(logtest servercc_lib)
target_link_libraries(logtest servercc_lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)