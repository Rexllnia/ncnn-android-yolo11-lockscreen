#include "yolov8_light_on_off.h"
#include "../ffmpeg_rtsp_client/ffmpeg_rtsp_client.h"
#include <jni.h>

double calculate_average_brightness(cv::Mat rgb_mat) {
    // 将图像转换为灰度图
    cv::Mat gray;
    cv::cvtColor(rgb_mat, gray, cv::COLOR_BGR2GRAY);

    // 计算整个灰度图的平均亮度
    cv::Scalar mean = cv::mean(gray);
    return mean[0];
}
void yolov8_frame_handle(AVCodecContext* codec_ctx,AVFrame *rgb_frame) {
    cv::Mat rgb_mat(codec_ctx->height, codec_ctx->width, CV_8UC3, rgb_frame->data[0], rgb_frame->linesize[0]);
    cv::Mat rgb_mat_copy = (rgb_frame->linesize[0] != codec_ctx->width*3) ? rgb_mat.clone() : rgb_mat;

    std::vector<Object> objects;


    double average_brightness = calculate_average_brightness(rgb_mat_copy);
    if (g_yolov8) {
        g_yolov8->detect(rgb_mat_copy,objects);
        ((YOLOv8_det_light_on_off *)g_yolov8)->update_objects_resource(objects,average_brightness);
    }

}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_ncnn_1android_1yolov8_1new_Yolov8Ncnn_openRTSP(JNIEnv *env, jobject thiz) {

    ffmpeg_rtsp_client_init(yolov8_frame_handle);
    ffmpeg_rtsp_client_start();
    return JNI_TRUE;
}