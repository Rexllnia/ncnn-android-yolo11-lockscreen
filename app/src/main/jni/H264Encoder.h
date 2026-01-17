#pragma once

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <android/log.h>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <cstring>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "H264_AMC", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "H264_AMC", __VA_ARGS__)

struct H264Packet {
    std::vector<uint8_t> data;
    uint64_t timestamp;
};

struct EncoderInputFrame {
    std::vector<uint8_t> nv21;
    int width;
    int height;
};

using H264Callback = std::function<void(H264Packet&&)>;

class H264Encoder {
public:
    explicit H264Encoder(H264Callback cb)
            : callback_(std::move(cb)) {
        worker_ = std::thread(&H264Encoder::loop, this);
    }

    ~H264Encoder() {
        stop();
    }

    void PushFrame(const uint8_t* nv21, int width, int height) {
        std::lock_guard<std::mutex> lk(mutex_);
        EncoderInputFrame f;
        f.width = width;
        f.height = height;
        f.nv21.assign(nv21, nv21 + width * height * 3 / 2); // NV21
        queue_.push(std::move(f));
        cond_.notify_one();
    }

private:
    void stop() {
        running_ = false;
        cond_.notify_all();
        if (worker_.joinable()) worker_.join();

        if (codec_) AMediaCodec_stop(codec_);
        if (codec_) AMediaCodec_delete(codec_);
        if (format_) AMediaFormat_delete(format_);
    }

    void initCodec(int width, int height) {
        if (codec_) return;

        width_ = width;
        height_ = height;
        fps_ = 30;
        bitrate_ = 2 * 1024 * 1024; // 默认 2Mbps

        codec_ = AMediaCodec_createEncoderByType("video/avc");
        if (!codec_) {
            LOGE("Failed to create MediaCodec");
            return;
        }

        format_ = AMediaFormat_new();
        AMediaFormat_setString(format_, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(format_, AMEDIAFORMAT_KEY_WIDTH, width_);
        AMediaFormat_setInt32(format_, AMEDIAFORMAT_KEY_HEIGHT, height_);
        AMediaFormat_setInt32(format_, AMEDIAFORMAT_KEY_BIT_RATE, bitrate_);
        AMediaFormat_setInt32(format_, AMEDIAFORMAT_KEY_FRAME_RATE, fps_);
        AMediaFormat_setInt32(format_, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);
        AMediaFormat_setInt32(format_, AMEDIAFORMAT_KEY_COLOR_FORMAT,
                              21 /* COLOR_FormatYUV420SemiPlanar */);

        if (AMediaCodec_configure(codec_, format_, nullptr, nullptr,
                                  AMEDIACODEC_CONFIGURE_FLAG_ENCODE) != AMEDIA_OK) {
            LOGE("MediaCodec configure failed");
            return;
        }

        AMediaCodec_start(codec_);
        running_ = true;
    }

    void loop() {
        while (true) {
            EncoderInputFrame frame;
            {
                std::unique_lock<std::mutex> lk(mutex_);
                cond_.wait(lk, [&] { return !queue_.empty() || !running_; });
                if (!running_ && queue_.empty()) break;

                if (queue_.empty()) continue;
                frame = std::move(queue_.front());
                queue_.pop();
            }

            // **第一次收到帧再初始化 codec**
            if (!codec_) {
                initCodec(frame.width, frame.height);
                if (!codec_) continue; // 初始化失败
            }

            feedInput(frame);
            drainOutput();
        }
    }

    void feedInput(const EncoderInputFrame& frame) {
        ssize_t idx = AMediaCodec_dequeueInputBuffer(codec_, 10000);
        if (idx < 0) return;

        size_t bufSize;
        uint8_t* buf = AMediaCodec_getInputBuffer(codec_, idx, &bufSize);
        if (!buf || frame.nv21.size() > bufSize) return;

        memcpy(buf, frame.nv21.data(), frame.nv21.size());

        uint64_t pts = pts_++ * 1000000 / fps_;
        AMediaCodec_queueInputBuffer(codec_, idx, 0, frame.nv21.size(), pts, 0);
    }

    void drainOutput() {
        AMediaCodecBufferInfo info;
        ssize_t idx;
        while ((idx = AMediaCodec_dequeueOutputBuffer(codec_, &info, 0)) >= 0) {
            size_t size;
            uint8_t* buf = AMediaCodec_getOutputBuffer(codec_, idx, &size);
            if (buf && info.size > 0 && callback_) {
                H264Packet pkt;
                pkt.timestamp = nowMs();
                pkt.data.assign(buf + info.offset,
                                buf + info.offset + info.size);
                callback_(std::move(pkt));
            }
            AMediaCodec_releaseOutputBuffer(codec_, idx, false);
        }
    }

    static uint64_t nowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
    }

private:
    AMediaCodec* codec_ = nullptr;
    AMediaFormat* format_ = nullptr;

    std::thread worker_;
    std::queue<EncoderInputFrame> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool running_ = false;

    int width_ = 0;
    int height_ = 0;
    int fps_ = 0;
    int bitrate_ = 0;
    uint64_t pts_ = 0;

    H264Callback callback_;
};
