//
// Created by zhengyufan on 2026/1/5.
//

#ifndef NCNN_ANDROID_YOLOV8_NEW_H264RINGBUFFER_H
#define NCNN_ANDROID_YOLOV8_NEW_H264RINGBUFFER_H

#include <queue>
#include <mutex>
#include <condition_variable>

struct H264Frame {
    std::vector<uint8_t> data;
    uint64_t timestamp;
};

class H264RingBuffer {
public:
    explicit H264RingBuffer(size_t max_frames = 30)
            : max_frames_(max_frames) {}

    // Camera 线程调用
    void Push(uint8_t* data, size_t size, uint64_t ts) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (queue_.size() >= max_frames_) {
            // 丢最旧帧（实时系统一定要这样）
            queue_.pop();
        }

        H264Frame frame;
        frame.data.assign(data, data + size);
        frame.timestamp = ts;
        queue_.push(std::move(frame));

        cond_.notify_one();
    }

    // RTSP 线程调用
    bool Pop(H264Frame& frame) {
        std::unique_lock<std::mutex> lock(mutex_);

        cond_.wait(lock, [&] {
            return !queue_.empty() || stop_;
        });

        if (stop_) return false;

        frame = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void Stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
        cond_.notify_all();
    }

private:
    std::queue<H264Frame> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    size_t max_frames_;
    bool stop_ = false;
};


#endif //NCNN_ANDROID_YOLOV8_NEW_H264RINGBUFFER_H
