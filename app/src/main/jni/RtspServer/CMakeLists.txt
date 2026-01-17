cmake_minimum_required(VERSION 3.10)
project(rtsp CXX)

# ===============================
# 基本配置
# ===============================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# ===============================
# 源文件
# ===============================
file(GLOB NET_SRC src/net/*.cpp)
file(GLOB XOP_SRC src/xop/*.cpp)

set(RTSP_SRC
        ${NET_SRC}
        ${XOP_SRC}
)

# ===============================
# 静态库
# ===============================
add_library(rtsp STATIC ${RTSP_SRC})

target_include_directories(rtsp PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/net>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/xop>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/3rdpart>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtsp
        PUBLIC
        dl
        m
)

set_target_properties(rtsp PROPERTIES
        OUTPUT_NAME rtsp
)

# ===============================
# 示例程序（可选）
# ===============================
option(BUILD_EXAMPLES "Build examples" OFF)

if(BUILD_EXAMPLES)
    add_executable(rtsp_server example/rtsp_server.cpp)
    target_link_libraries(rtsp_server rtsp)

    add_executable(rtsp_pusher example/rtsp_pusher.cpp)
    target_link_libraries(rtsp_pusher rtsp)

    add_executable(rtsp_h264_file example/rtsp_h264_file.cpp)
    target_link_libraries(rtsp_h264_file rtsp)
endif()
