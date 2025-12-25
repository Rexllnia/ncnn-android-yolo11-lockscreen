#ifndef SIMPLE_KALMAN_H
#define SIMPLE_KALMAN_H

#ifdef __cplusplus
extern "C" {
#endif

#define KALMAN_STATE_DIM 8
#define KALMAN_MEAS_DIM  4

typedef struct {
    float x[KALMAN_STATE_DIM];                 // state
    float P[KALMAN_STATE_DIM][KALMAN_STATE_DIM]; // covariance
} KalmanState;

/* 初始化 */
void kalman_init(KalmanState* kf,
                 float cx, float cy, float w, float h);

/* 预测 */
void kalman_predict(KalmanState* kf);

/* 更新 */
void kalman_update(KalmanState* kf,
                   float cx, float cy, float w, float h);

#ifdef __cplusplus
}
#endif

#endif
