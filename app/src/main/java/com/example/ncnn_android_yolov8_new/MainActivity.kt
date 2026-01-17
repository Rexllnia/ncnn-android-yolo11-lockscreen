package com.example.ncnn_android_yolov8_new

import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.core.app.ActivityCompat
import com.example.ncnn_android_yolov8_new.ui.theme.Ncnnandroidyolov8_newTheme
import android.graphics.PixelFormat
import com.example.ncnn_android_yolov8_new.com.example.ncnn_android_yolov8_new.ApiServer

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.os.PowerManager
import androidx.core.app.NotificationCompat
import androidx.core.content.ContextCompat

class MainActivity : ComponentActivity() {
//    private val yolov8ncnn = Yolov8Ncnn()
//    private var current_model = 0
//    private var current_cpugpu = 1
//    private var cameraView: SurfaceView? = null
//    private val apiServer = ApiServer()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
//        yolo_module_test();
//        reload()
//        setContentView(R.layout.main)
//
//        // 初始化 SurfaceView
//        val cameraView: SurfaceView = findViewById(R.id.cameraview)
//        val surfaceHolder = cameraView.holder
//
//        // 设置 SurfaceView 相关配置
//        surfaceHolder.setFormat(PixelFormat.RGBA_8888)
//
//        // 启动后台服务并将 Surface 传递过去
//        val intent = Intent(this, MyService::class.java)
//        intent.putExtra("surface", surfaceHolder.surface)
//
        startService(Intent(this,MyService::class.java))

//        yolov8ncnn.openRTSP()
//        apiServer.startServer()
    }
//    private fun yolo_module_test() {
//        setContentView(R.layout.main)
//
//        cameraView = findViewById<View>(R.id.cameraview) as SurfaceView
//        cameraView!!.holder.setFormat(PixelFormat.RGBA_8888);
//        cameraView!!.holder.addCallback(object : SurfaceHolder.Callback {
//            override fun surfaceCreated(holder: SurfaceHolder) {
//                initCamera(holder)
//            }
//            override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
//                yolov8ncnn.setOutputWindow(holder.surface)
//            }
//            override fun surfaceDestroyed(holder: SurfaceHolder) {
//                // TODO: closeCamera() if needed
//            }
//        })
//        requestCamera()
//    }
//    private fun requestCamera() {
//        if (ActivityCompat.checkSelfPermission(
//                this,
//                android.Manifest.permission.CAMERA
//            ) != PackageManager.PERMISSION_GRANTED
//        ) {
//            ActivityCompat.requestPermissions(this, arrayOf(android.Manifest.permission.CAMERA), 100)
//        }
//    }
//    private fun reload() {
//        val ret_init: Boolean = yolov8ncnn.loadModel(assets,0, current_model, current_cpugpu)
//        if (!ret_init) {
//            Log.e("MainActivity", "yolov8ncnn loadModel failed")
//        }
//    }
//    private fun initCamera(holder: SurfaceHolder) {
//        yolov8ncnn.setOutputWindow(holder.surface)
//
//        val ret = yolov8ncnn.openCamera(1) // 1 后置摄像头
//        if (!ret) {
//            Log.e("MainActivity", "Camera open failed")
//        }
//    }
}
