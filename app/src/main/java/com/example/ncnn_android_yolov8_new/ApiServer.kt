package com.example.ncnn_android_yolov8_new.com.example.ncnn_android_yolov8_new

class ApiServer {
    external fun startServer(): Boolean
    val serverStatus: String?
        external get

    companion object {
        init {
            System.loadLibrary("apiServer")
        }
    }
}