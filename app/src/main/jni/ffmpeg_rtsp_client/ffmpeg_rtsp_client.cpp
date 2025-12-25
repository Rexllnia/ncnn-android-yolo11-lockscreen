#include "ffmpeg_rtsp_client.h"

static frame_handle_t g_singleton_frame_handle;
void decode_rtsp(const char* url) {
    AVFormatContext* fmt_ctx = nullptr;

    // 打开 RTSP 流
    if (avformat_open_input(&fmt_ctx, url, nullptr, nullptr) != 0) {
        std::cerr << "无法打开 RTSP 流: " << url << std::endl;
        return;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "无法获取流信息" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    int video_stream_index = -1;
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "没有找到视频流" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVCodecParameters* codec_par = fmt_ctx->streams[video_stream_index]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codec_par->codec_id);
    if (!codec) {
        std::cerr << "无法找到解码器" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "无法创建解码上下文" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    if (avcodec_parameters_to_context(codec_ctx, codec_par) < 0) {
        std::cerr << "无法将 codec 参数拷贝到 codec context" << std::endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        std::cerr << "无法打开解码器" << std::endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    struct SwsContext* sws_ctx = nullptr;
    sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                             codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
                             SWS_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t* rgb_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1));
    AVFrame* rgb_frame = av_frame_alloc();
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, rgb_buffer, AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_index) {
            if (avcodec_send_packet(codec_ctx, pkt) == 0) {
                while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    // 转换为 RGB
                    sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height,
                              rgb_frame->data, rgb_frame->linesize);
                    g_singleton_frame_handle(codec_ctx,rgb_frame);
                }
            }
        }
        av_packet_unref(pkt);
    }

    // 清理
    av_free(rgb_buffer);
    av_frame_free(&rgb_frame);
    sws_freeContext(sws_ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
}

static pthread_t rtsp_client_thread;
void* rtsp_loop(void* arg){
    const char* rtsp_url = "rtsp://127.0.0.1:8080/h264.sdp";
    decode_rtsp(rtsp_url);
    return (void*)0;
}

void ffmpeg_rtsp_client_init(frame_handle_t frame_handle)
{
    g_singleton_frame_handle = frame_handle;
}

void ffmpeg_rtsp_client_start() {
    pthread_create(&rtsp_client_thread, NULL, rtsp_loop,NULL);

}

