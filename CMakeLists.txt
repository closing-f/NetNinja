cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
project(test)
# include_directories("include/logger.h")
# set(LIB_SRC
#     logger/logger.cpp
#     utils.cpp)

add_library(servercc SHARED src/logger.cpp src/utils.cpp src/config.cpp src/mutex.cpp src/thread.cpp src/fiber.cpp src/scheduler.cpp src/iomanager.cpp src/timer.cpp src/hook.cpp src/fd_manager.cpp src/address.cpp src/socket.cpp src/bytearray.cpp 
src/http/http.cpp src/http/http11_parser.cc src/http/httpclient_parser.rl.cc src/http/http_parser.cpp src/tcpserver.cpp src/http_connection.cpp src/http_server.cpp src/http_servlet.cpp src/http_session.cpp src/stream.cpp src/socket_stream.cpp
src/uri.rl.cc)
# add_subdirectory(logger)

include_directories(/usr/local/include/yaml-cpp)
# test.cpp代表你的测试文件
# add_executable(test_xx test_http_parser.cpp)
add_executable(test test_config.cpp)
add_dependencies(test servercc)
target_link_libraries(test servercc libyaml-cpp.so pthread dl)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)