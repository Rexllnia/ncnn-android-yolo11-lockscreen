#include "api_server.h"
#include <jni.h>


static pthread_t server_thread;
// JNI方法实现：获取本地IP地址
JNIEXPORT jstring JNICALL
Java_com_example_ncnn_1android_1yolov8_1new_ApiServer_getServerStatus(JNIEnv* env, jobject thiz) {
   char* ips = get_local_ips();
   return (*env)->NewStringUTF(env, ips);
}

// JNI方法实现：启动服务器
JNIEXPORT jboolean
JNICALL
Java_com_example_ncnn_1android_1yolov8_1new_com_example_ncnn_1android_1yolov8_1new_ApiServer_startServer(JNIEnv *env, jobject thiz) {
   // 创建服务器线程


    if (pthread_create(&server_thread, NULL, server_loop,NULL) != 0) {
       return JNI_FALSE;
   }


   return JNI_TRUE;
}

