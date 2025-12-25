package com.example.ncnn_android_yolov8_new

import android.app.Service
import android.content.Intent
import android.graphics.PixelFormat
import android.os.IBinder
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import com.example.ncnn_android_yolov8_new.com.example.ncnn_android_yolov8_new.ApiServer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.os.Build
import androidx.core.app.NotificationCompat

class MyService : Service() {
    private val yolov8ncnn = Yolov8Ncnn()
    private val apiServer = ApiServer()
    private val CHANNEL_ID = "my_foreground_service_channel"

    override fun onBind(intent: Intent): IBinder {
        TODO("Return the communication channel to the service.")
    }
    override fun onCreate() {
        super.onCreate()

        // 创建通知频道 (适用于 Android 8.0 及以上)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "My Foreground Service Channel"
            val importance = NotificationManager.IMPORTANCE_DEFAULT
            val channel = NotificationChannel(CHANNEL_ID, name, importance)
            val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }

        // 创建通知
        val notification: Notification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Foreground Service")
            .setContentText("This service is running in the foreground")
            .setSmallIcon(R.drawable.ic_launcher_background)  // 使用你的图标资源
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .build()

        // 启动前台服务
        startForeground(1, notification)

        // 执行你的任务 (例如摄像头处理等)
    }
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
//        val surface: Surface? = intent?.getParcelableExtra("surface")
        yolov8ncnn.loadModel(assets,0, 0, 1)
        // 检查 Surface 是否有效
//        surface?.let {
            // 使用 Surface 进行渲染或图像处理
//        yolov8ncnn.setOutputWindow(null)
//        }

        yolov8ncnn.openCamera(1) // 1 后置摄像头
        apiServer.startServer()
        return START_STICKY
    }

}