project(apiServer)

cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID)
    # 当前是原生操作系统（例如 Linux、Windows、macOS）构建
    add_library(apiServer SHARED ./api_server.c)
else()
    # 当前是 Android/JNI 构建，可以定义其他宏或进行其他配置
    add_library(apiServer SHARED ./api_server.c ./jni_export.c)

endif()
target_compile_options(apiServer PRIVATE -O0)
target_link_libraries(apiServer)  # API服务器不依赖其他第三方库