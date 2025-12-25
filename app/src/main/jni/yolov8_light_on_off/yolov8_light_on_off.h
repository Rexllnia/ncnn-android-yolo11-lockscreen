#ifndef NCNN_ANDROID_YOLOV8_NEW_YOLOV8_LIGHT_ON_OFF_H
#define NCNN_ANDROID_YOLOV8_NEW_YOLOV8_LIGHT_ON_OFF_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "json.hpp"
#include "../yolov8.h"
#include "../sort.h"
#include <jni.h>


extern "C" {
    #include <kvlist.h>
}

class YOLOv8_det_light_on_off : public YOLOv8_det
{


public:
    YOLOv8_det_light_on_off();
    virtual int draw(cv::Mat& rgb, const std::vector<Object>& objects);
    virtual void update_objects_resource(const std::vector<Object>& objects,double average_brightness);
};


#endif //NCNN_ANDROID_YOLOV8_NEW_YOLOV8_LIGHT_ON_OFF_H
