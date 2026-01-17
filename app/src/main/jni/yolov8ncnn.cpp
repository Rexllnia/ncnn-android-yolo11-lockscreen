// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#include <android/log.h>

#include <jni.h>

#include <string>
#include <vector>

#include <platform.h>
#include <benchmark.h>

#include "yolov8.h"
#include "yolov8_light_on_off/yolov8_light_on_off.h"
#include "ndkcamera.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include "./RtspServer/src/xop/RtspServer.h"
//#include "./RtspServer/src/net/Timer.h"
//#include "H264RingBuffer.h"
#include <thread>
#include <memory>
#include <iostream>
#include <string>
#include "watchdog.h"
#if __ARM_NEON
#include <arm_neon.h>
#endif // __ARM_NEON

//extern H264RingBuffer* g_ring;


static int draw_unsupported(cv::Mat& rgb)
{
    const char text[] = "unsupported";

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 1.0, 1, &baseLine);

    int y = (rgb.rows - label_size.height) / 2;
    int x = (rgb.cols - label_size.width) / 2;

    cv::rectangle(rgb, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                    cv::Scalar(255, 255, 255), -1);

    cv::putText(rgb, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0));

    return 0;
}

static int draw_fps(cv::Mat& rgb)
{
    // resolve moving average
    float avg_fps = 0.f;
    {
        static double t0 = 0.f;
        static float fps_history[10] = {0.f};

        double t1 = ncnn::get_current_time();
        if (t0 == 0.f)
        {
            t0 = t1;
            return 0;
        }

        float fps = 1000.f / (t1 - t0);
        t0 = t1;

        for (int i = 9; i >= 1; i--)
        {
            fps_history[i] = fps_history[i - 1];
        }
        fps_history[0] = fps;

        if (fps_history[9] == 0.f)
        {
            return 0;
        }

        for (int i = 0; i < 10; i++)
        {
            avg_fps += fps_history[i];
        }
        avg_fps /= 10.f;
    }

    char text[32];
    sprintf(text, "FPS=%.2f", avg_fps);

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int y = 0;
    int x = rgb.cols - label_size.width;

    cv::rectangle(rgb, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                    cv::Scalar(255, 255, 255), -1);

    cv::putText(rgb, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

    return 0;
}

YOLOv8* g_yolov8 = 0;
static ncnn::Mutex lock;

class MyNdkCamera : public NdkCameraWindow
{
public:
    virtual void on_image_render(cv::Mat& rgb) const;
};

void MyNdkCamera::on_image_render(cv::Mat& rgb) const
{
    // yolov8
    {
        ncnn::MutexLockGuard g(lock);

        if (g_yolov8)
        {
            std::vector<Object> objects;
            g_yolov8->detect(rgb, objects);

            g_yolov8->draw(rgb, objects);
        }
        else
        {
            draw_unsupported(rgb);
        }
    }

//    draw_fps(rgb);
}

static MyNdkCamera* g_camera = 0;


extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "JNI_OnLoad");

    g_camera = new MyNdkCamera;

    ncnn::create_gpu_instance();

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "JNI_OnUnload");

    {
        ncnn::MutexLockGuard g(lock);

        delete g_yolov8;
        g_yolov8 = 0;
    }

    ncnn::destroy_gpu_instance();

    delete g_camera;
    g_camera = 0;
}

// public native boolean loadModel(AssetManager mgr, int taskid, int modelid, int cpugpu);
JNIEXPORT jboolean JNICALL Java_com_example_ncnn_1android_1yolov8_1new_Yolov8Ncnn_loadModel(JNIEnv* env, jobject thiz, jobject assetManager, jint taskid, jint modelid, jint cpugpu)
{
//    if (taskid < 0 || taskid > 5 || modelid < 0 || modelid > 8 || cpugpu < 0 || cpugpu > 2)
//    {
//        return JNI_FALSE;
//    }

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "loadModel %p", mgr);

    const char* tasknames[7] =
    {
        "",
        "_oiv7",
        "_seg",
        "_pose",
        "_cls",
        "_obb",
        ""
    };

    const char* modeltypes[9] =
    {
        "n",
        "s",
        "m",
        "n",
        "s",
        "m",
        "n",
        "s",
        "m"
    };

    std::string parampath = std::string("yolov8") + modeltypes[(int)modelid] + tasknames[(int)taskid] + ".ncnn.param";
    std::string modelpath = std::string("yolov8") + modeltypes[(int)modelid] + tasknames[(int)taskid] + ".ncnn.bin";
    bool use_gpu = (int)cpugpu == 1;
    bool use_turnip = (int)cpugpu == 2;

    // reload
    {
        ncnn::MutexLockGuard g(lock);

        {
            static int old_taskid = 0;
            static int old_modelid = 0;
            static int old_cpugpu = 0;
            if (taskid != old_taskid || (modelid % 3) != old_modelid || cpugpu != old_cpugpu)
            {
                // taskid or model or cpugpu changed
                delete g_yolov8;
                g_yolov8 = 0;
            }
            old_taskid = taskid;
            old_modelid = modelid % 3;
            old_cpugpu = cpugpu;

            ncnn::destroy_gpu_instance();

            if (use_turnip)
            {
                ncnn::create_gpu_instance("libvulkan_freedreno.so");
            }
            else if (use_gpu)
            {
                ncnn::create_gpu_instance();
            }

            if (!g_yolov8)
            {
                if (taskid == 0) g_yolov8 = new YOLOv8_det_coco;
                if (taskid == 1) g_yolov8 = new YOLOv8_det_oiv7;
                if (taskid == 2) g_yolov8 = new YOLOv8_seg;
                if (taskid == 3) g_yolov8 = new YOLOv8_pose;
                if (taskid == 4) g_yolov8 = new YOLOv8_cls;
                if (taskid == 5) g_yolov8 = new YOLOv8_obb;
                if (taskid == 6) g_yolov8 = new YOLOv8_det_light_on_off;

                g_yolov8->load(mgr, parampath.c_str(), modelpath.c_str(), use_gpu || use_turnip);
            }
            int target_size = 320;
            if ((int)modelid >= 3)
                target_size = 480;
            if ((int)modelid >= 6)
                target_size = 640;
            g_yolov8->set_det_target_size(target_size);
        }
    }

    return JNI_TRUE;
}
void camera_restart_cb(void* user) {
    __android_log_print(ANDROID_LOG_ERROR, "Watchdog", "Camera timeout! Restarting...");
    // user 可以是你的 camera 对象
    g_camera->close();
    g_camera->open((int)1);
}
// public native boolean openCamera(int facing);
JNIEXPORT jboolean JNICALL Java_com_example_ncnn_1android_1yolov8_1new_Yolov8Ncnn_openCamera(JNIEnv* env, jobject thiz, jint facing)
{
    if (facing < 0 || facing > 1)
        return JNI_FALSE;

    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "openCamera %d", facing);

    g_camera->open((int)facing);
    watchdog_get_instance(10000, camera_restart_cb, NULL); // 10秒超时
    return JNI_TRUE;
}

// public native boolean closeCamera();
JNIEXPORT jboolean JNICALL Java_com_example_ncnn_1android_1yolov8_1new_Yolov8Ncnn_closeCamera(JNIEnv* env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "closeCamera");

    g_camera->close();

    return JNI_TRUE;
}




class H264File
{
public:
    H264File(int buf_size=500000);
    ~H264File();

    bool Open(const char *path);
    void Close();

    bool IsOpened() const
    { return (m_file != NULL); }

    int ReadFrame(char* in_buf, int in_buf_size, bool* end);

private:
    FILE *m_file = NULL;
    char *m_buf = NULL;
    int  m_buf_size = 0;
    int  m_bytes_used = 0;
    int  m_count = 0;
};

//void SendFrameThread(xop::RtspServer* rtsp_server, xop::MediaSessionId session_id);

// public native boolean setOutputWindow(Surface surface);

JNIEXPORT jboolean JNICALL Java_com_example_ncnn_1android_1yolov8_1new_Yolov8Ncnn_setOutputWindow(JNIEnv* env, jobject thiz, jobject surface)
{

//    g_ring = new H264RingBuffer(60); // 2 秒缓冲
//
//    std::string suffix = "live";
//    std::string ip = "127.0.0.1";
//    std::string port = "554";
//    std::string rtsp_url = "rtsp://" + ip + ":" + port + "/" + suffix;

//    std::shared_ptr<xop::EventLoop> event_loop(new xop::EventLoop());
//    std::shared_ptr<xop::RtspServer> server = xop::RtspServer::Create(event_loop.get());

//    if (!server->Start("0.0.0.0", atoi(port.c_str()))) {
//        printf("RTSP Server listen on %s failed.\n", port.c_str());
//        return 0;
//    }
//
//#ifdef AUTH_CONFIG
//    server->SetAuthConfig("-_-", "admin", "12345");
//#endif
//
//    xop::MediaSession *session = xop::MediaSession::CreateNew("live");
//    session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
//    //session->StartMulticast();
//    session->AddNotifyConnectedCallback([] (xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port){
//        printf("RTSP client connect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
//    });
//
//    session->AddNotifyDisconnectedCallback([](xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port) {
//        printf("RTSP client disconnect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
//    });
//
//    xop::MediaSessionId session_id = server->AddSession(session);
//
//    std::thread t1(SendFrameThread, server.get(), session_id);
//    t1.detach();
//
//    std::cout << "Play URL: " << rtsp_url << std::endl;

    return JNI_TRUE;
}

}
//
//void SendFrameThread(xop::RtspServer* rtsp_server, xop::MediaSessionId session_id)
//{
//    int buf_size = 2000000;
//    std::unique_ptr<uint8_t> frame_buf(new uint8_t[buf_size]);
//
//    while(1) {
//        H264Frame frame;
//        if (!g_ring->Pop(frame)) {
//            continue;
//        }
//
//        xop::AVFrame videoFrame = {0};
//        videoFrame.type = 0;
//        videoFrame.size = frame.data.size();
//        videoFrame.timestamp = xop::H264Source::GetTimestamp();
//
//        videoFrame.buffer.reset(new uint8_t[videoFrame.size]);
//        memcpy(videoFrame.buffer.get(),
//               frame.data.data(),
//               videoFrame.size);
//
//        rtsp_server->PushFrame(session_id,
//                               xop::channel_0,
//                               videoFrame);
//    };
//}
#include "./yolov8_light_on_off/yolov8ncnn_extend.cpp"


