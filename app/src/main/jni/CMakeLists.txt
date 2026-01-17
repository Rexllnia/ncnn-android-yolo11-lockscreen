project(yolov8ncnn)

cmake_minimum_required(VERSION 3.10)

set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-mobile-4.12.0-android/sdk/native/jni)
find_package(OpenCV REQUIRED core imgproc)

set(ncnn_DIR ${CMAKE_SOURCE_DIR}/ncnn-20250916-android-vulkan/${ANDROID_ABI}/lib/cmake/ncnn)

find_package(ncnn REQUIRED)

# FFmpeg 路径
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/FFmpeg)



# 引入 FFmpeg
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
        ${FFMPEG_DIR}/${ANDROID_ABI}/lib/libavcodec.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
        ${FFMPEG_DIR}/${ANDROID_ABI}/lib/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
        ${FFMPEG_DIR}/${ANDROID_ABI}/lib/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
        ${FFMPEG_DIR}/${ANDROID_ABI}/lib/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
        ${FFMPEG_DIR}/${ANDROID_ABI}/lib/libswscale.so)

# 包含 FFmpeg 头文件路径
include_directories(${FFMPEG_DIR}/${ANDROID_ABI}/include)


set(LIBUBOX_DIR ${CMAKE_SOURCE_DIR}/libubox-2015-11-08)
include_directories(${LIBUBOX_DIR})


add_subdirectory(rest_api_server)
add_subdirectory(libubox-2015-11-08)
#add_subdirectory(RtspServer)
add_subdirectory(mosquitto)
include_directories(mosquitto/include)
#add_subdirectory(RtspServer)
#set(MQTT_DIR ${CMAKE_SOURCE_DIR}/mosquitto)
#add_library(mosquitto STATIC IMPORTED)
#set_target_properties(mosquitto PROPERTIES
#        IMPORTED_LOCATION
#        ${MQTT_DIR}/android_out/${ANDROID_ABI}/libmosquitto.so
#)
#include_directories(${MQTT_DIR}/include)
add_library(yolov8ncnn SHARED yolov8ncnn.cpp  yolov8.cpp
        yolov8_cls.cpp
        yolov8_det.cpp
        yolov8_obb.cpp
        yolov8_seg.cpp
        yolov8_pose.cpp
        yolov8_light_on_off/yolov8_light_on_off.cpp
        sort.cpp
        kalman.c
        ffmpeg_rtsp_client/ffmpeg_rtsp_client.cpp
        ndkcamera.cpp
        tcp_client.c
        spsc.c
        watchdog.c
)

target_link_libraries(yolov8ncnn ncnn ${OpenCV_LIBS} camera2ndk mediandk apiServer avcodec
        avformat
        avutil
        swresample
        swscale
        ubox
        libmosquitto
#        rtsp
)
