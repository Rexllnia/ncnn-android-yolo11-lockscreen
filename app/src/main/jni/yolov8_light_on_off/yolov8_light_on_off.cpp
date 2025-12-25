#include "yolov8_light_on_off.h"

using json = nlohmann::json;


YOLOv8_det_light_on_off::YOLOv8_det_light_on_off() {

}

static SORT g_sort;   // ❗全局或 main 外，不能每帧创建

int YOLOv8_det_light_on_off::draw(cv::Mat& rgb, const std::vector<Object>& objects)
{
    std::vector<Detection> dets;
    for (const auto& obj : objects) {
        if (obj.prob < 0.25f) continue;

        Detection d;
        d.x1 = obj.rect.x;
        d.y1 = obj.rect.y;
        d.x2 = obj.rect.x + obj.rect.width;
        d.y2 = obj.rect.y + obj.rect.height;
        d.conf = obj.prob;
        d.cls  = obj.label;
        dets.push_back(d);
    }

    std::vector<TrackedDetection> tracks = g_sort.update(dets);
    for (const auto& t : tracks) {
        cv::rectangle(rgb,
                      cv::Point(t.x1, t.y1),
                      cv::Point(t.x2, t.y2),
                      cv::Scalar(0, 255, 0), 2);

        char text[64];
        memset(text,0,sizeof (text));
        sprintf(text, "ID:%d %.2f class %d", t.id, t.conf,t.cls);

//        cv::putText(rgb, text,
//                    cv::Point(t.x1, t.y1 - 5),
//                    cv::FONT_HERSHEY_SIMPLEX,
//                    0.6, cv::Scalar(0, 255, 0), 2);
    }
    return 0;
}

extern "C" {
void api_server_set_content(const char* content);
}


void YOLOv8_det_light_on_off::update_objects_resource(const std::vector<Object>& objects,double average_brightness)
{

    std::vector<Detection> dets;
    for (const auto& obj : objects) {
        if (obj.prob < 0.25f) continue;

        Detection d;
        d.x1 = obj.rect.x;
        d.y1 = obj.rect.y;
        d.x2 = obj.rect.x + obj.rect.width;
        d.y2 = obj.rect.y + obj.rect.height;
        d.conf = obj.prob;
        d.cls  = obj.label;
        dets.push_back(d);
    }
    std::vector<TrackedDetection> tracks = g_sort.update(dets);

    auto it = std::max_element(
            tracks.begin(),
            tracks.end(),
            [](const TrackedDetection& a, const TrackedDetection& b) {
                return a.conf < b.conf;
            }
    );

    json jroot;
    if (it != tracks.end()) {
        jroot["status"] = it->cls;
    } else {
        jroot["status"] = 1;
    }



    std::string s = jroot.dump();
    __android_log_print(ANDROID_LOG_DEBUG, "light on off", "%s",s.c_str());
    api_server_set_content(s.c_str());
}



