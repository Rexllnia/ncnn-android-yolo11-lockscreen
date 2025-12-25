#ifndef NCNN_ANDROID_YOLOV8_NEW_FFMPEG_RTSP_CLIENT_H
#define NCNN_ANDROID_YOLOV8_NEW_FFMPEG_RTSP_CLIENT_H

#include <iostream>
#include <thread>
#include <chrono>

extern "C" {
#include "../rest_api_server/api_server.h"
#include "../FFmpeg/arm64-v8a/include/libavformat/avformat.h"
#include "../FFmpeg/arm64-v8a/include/libavcodec/avcodec.h"
#include "../FFmpeg/arm64-v8a/include/libswscale/swscale.h"
#include "../FFmpeg/arm64-v8a/include/libavutil/imgutils.h"
}

typedef void (*frame_handle_t) (AVCodecContext* codec_ctx,AVFrame *rgb_frame);

void ffmpeg_rtsp_client_init(frame_handle_t frame_handle);
void ffmpeg_rtsp_client_start();

#endif //NCNN_ANDROID_YOLOV8_NEW_FFMPEG_RTSP_CLIENT_H
