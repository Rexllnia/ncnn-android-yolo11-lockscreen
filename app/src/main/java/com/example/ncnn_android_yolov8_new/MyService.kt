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
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioTrack
import android.media.session.MediaSession
import android.media.session.PlaybackState
import android.os.Build
import android.os.PowerManager
import androidx.core.app.NotificationCompat
import android.app.Notification.MediaStyle

class MyService : Service() {
    private val yolov8ncnn = Yolov8Ncnn()
    private val apiServer = ApiServer()
    private val CHANNEL_ID = "my_foreground_service_channel"
    private lateinit var mediaSession: MediaSession
    private var wakeLock: PowerManager.WakeLock? = null
    private lateinit var audioManager: AudioManager
    private lateinit var audioTrack: AudioTrack

    override fun onBind(intent: Intent): IBinder {
        TODO("Return the communication channel to the service.")
    }
    override fun onCreate() {
        super.onCreate()

        val pm = getSystemService(Context.POWER_SERVICE) as PowerManager
        wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "MyApp::CameraWakeLock")
        wakeLock?.acquire() // CPU 持续唤醒

        // 创建通知频道 (适用于 Android 8.0 及以上)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "My Foreground Service Channel"
            val importance = NotificationManager.IMPORTANCE_DEFAULT
            val channel = NotificationChannel(CHANNEL_ID, name, importance)
            val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
        mediaSession = MediaSession(this, "CameraServiceSession")
        mediaSession.isActive = true // 激活，让系统认为我们是媒体应用
        // 创建通知
        val notification: Notification =
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                Notification.Builder(this, CHANNEL_ID)
                    .setContentTitle("Camera Service")
                    .setContentText("Running")
                    .setSmallIcon(R.drawable.ic_launcher_background)
                    .setStyle(
                        MediaStyle()
                            .setMediaSession(mediaSession.sessionToken)
                    )
                    .setOngoing(true)
                    .build()
            } else {
                NotificationCompat.Builder(this)
                    .setContentTitle("Camera Service")
                    .setContentText("Running")
                    .setSmallIcon(R.drawable.ic_launcher_background)
                    .setOngoing(true)
                    .build()
            }

        val state = PlaybackState.Builder()
            .setActions(
                PlaybackState.ACTION_PLAY or
                        PlaybackState.ACTION_PAUSE or
                        PlaybackState.ACTION_PLAY_PAUSE
            )
            .setState(
                PlaybackState.STATE_PLAYING,
                PlaybackState.PLAYBACK_POSITION_UNKNOWN,
                1.0f
            )
            .build()

        mediaSession.setPlaybackState(state)
        val sampleRate = 44100
        val bufferSize = AudioTrack.getMinBufferSize(
            sampleRate,
            AudioFormat.CHANNEL_OUT_MONO,
            AudioFormat.ENCODING_PCM_16BIT
        )

        audioTrack = AudioTrack(
            AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build(),
            AudioFormat.Builder()
                .setSampleRate(sampleRate)
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                .build(),
            bufferSize,
            AudioTrack.MODE_STREAM,
            AudioManager.AUDIO_SESSION_ID_GENERATE
        )

        audioTrack.play()

// 写入静音数据（循环线程）
        Thread {
            val silence = ByteArray(bufferSize)
            while (true) {
                audioTrack.write(silence, 0, silence.size)
                Thread.sleep(200)
            }
        }.start()
        // 执行你的任务 (例如摄像头处理等)



        yolov8ncnn.loadModel(assets,0, 0, 1)
        yolov8ncnn.openCamera(1)
        apiServer.startServer()

        // 启动前台服务
        startForeground(1, notification)
    }
}