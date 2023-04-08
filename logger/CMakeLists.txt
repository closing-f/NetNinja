# cmake_minimum_required(VERSION 2.8)

# project(hello)

# add_executable(hello main.cpp)
# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

cmake_minimum_required(VERSION 2.8)

project(logger)

add_library(logger STATIC logger.cpp)


install(TARGETS logger DESTINATION lib)
