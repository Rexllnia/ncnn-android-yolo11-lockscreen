#include "kalman.h"
#include <string.h>

static const float Q = 1e-2f;  // process noise
static const float R = 1e-1f;  // measurement noise

void kalman_init(KalmanState* kf,
                 float cx, float cy, float w, float h)
{
    memset(kf, 0, sizeof(KalmanState));

    kf->x[0] = cx;
    kf->x[1] = cy;
    kf->x[2] = w;
    kf->x[3] = h;

    for (int i = 0; i < KALMAN_STATE_DIM; i++)
        kf->P[i][i] = 1.0f;
}

/* x = F x */
void kalman_predict(KalmanState* kf)
{
    for (int i = 0; i < 4; i++)
        kf->x[i] += kf->x[i + 4];

    /* P = F P Fᵀ + Q */
    for (int i = 0; i < KALMAN_STATE_DIM; i++)
        kf->P[i][i] += Q;
}

/* 标准 KF 更新（已手推简化） */
void kalman_update(KalmanState* kf,
                   float cx, float cy, float w, float h)
{
    float z[KALMAN_MEAS_DIM] = {cx, cy, w, h};

    for (int i = 0; i < KALMAN_MEAS_DIM; i++) {
        float y = z[i] - kf->x[i];               // innovation
        float S = kf->P[i][i] + R;                // innovation cov
        float K = kf->P[i][i] / S;                // Kalman gain

        kf->x[i] += K * y;
        kf->P[i][i] *= (1.0f - K);
    }
}
