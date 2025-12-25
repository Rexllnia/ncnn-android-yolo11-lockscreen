#pragma once
#include <vector>

extern "C" {
#include "kalman.h"
}

struct Detection {
    float x1, y1, x2, y2;
    float conf;
    int cls;
};

struct TrackedDetection : Detection {
    int id;
};

struct Track {
    KalmanState kf;
    int id;
    int missed;
};

class SORT {
public:
    SORT();
    std::vector<TrackedDetection> update(const std::vector<Detection>& detections);

private:
    float iou(const TrackedDetection& a, const Detection& b);

    std::vector<Track> tracks;
    int next_id;
    int max_missed;
};
