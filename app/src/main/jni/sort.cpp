#include "sort.h"
#include <algorithm>
#include <cmath>

using namespace std;

SORT::SORT() {
    next_id = 0;
    max_missed = 5;
}

static TrackedDetection kf_to_box(const Track& t) {
    float cx = t.kf.x[0];
    float cy = t.kf.x[1];
    float w  = t.kf.x[2];
    float h  = t.kf.x[3];

    TrackedDetection d;
    d.x1 = cx - w * 0.5f;
    d.y1 = cy - h * 0.5f;
    d.x2 = cx + w * 0.5f;
    d.y2 = cy + h * 0.5f;
    d.id = t.id;
    return d;
}

vector<TrackedDetection> SORT::update(const vector<Detection>& detections) {
    vector<TrackedDetection> output;

    // 1. predict
    vector<TrackedDetection> predicts;
    for (auto& t : tracks) {
        kalman_predict(&t.kf);
        t.missed++;
        predicts.push_back(kf_to_box(t));
    }

    vector<bool> det_used(detections.size(), false);

    // 2. greedy IOU match
    for (int i = 0; i < tracks.size(); i++) {
        float best_iou = 0.f;
        int best_j = -1;

        for (int j = 0; j < detections.size(); j++) {
            if (det_used[j]) continue;
            float v = iou(predicts[i], detections[j]);
            if (v > best_iou) {
                best_iou = v;
                best_j = j;
            }
        }

        if (best_iou > 0.3f) {
            det_used[best_j] = true;
            tracks[i].missed = 0;

            float cx = (detections[best_j].x1 + detections[best_j].x2) * 0.5f;
            float cy = (detections[best_j].y1 + detections[best_j].y2) * 0.5f;
            float w  = detections[best_j].x2 - detections[best_j].x1;
            float h  = detections[best_j].y2 - detections[best_j].y1;

            kalman_update(&tracks[i].kf, cx, cy, w, h);

            auto td = kf_to_box(tracks[i]);
            td.conf = detections[best_j].conf;
            td.cls  = detections[best_j].cls;
            output.push_back(td);
        }
    }

    // 3. new tracks
    for (int j = 0; j < detections.size(); j++) {
        if (!det_used[j]) {
            Track t;
            float cx = (detections[j].x1 + detections[j].x2) * 0.5f;
            float cy = (detections[j].y1 + detections[j].y2) * 0.5f;
            float w  = detections[j].x2 - detections[j].x1;
            float h  = detections[j].y2 - detections[j].y1;

            kalman_init(&t.kf, cx, cy, w, h);
            t.id = next_id++;
            t.missed = 0;
            tracks.push_back(t);

            TrackedDetection td = kf_to_box(t);
            td.conf = detections[j].conf;
            td.cls  = detections[j].cls;
            output.push_back(td);
        }
    }

    // 4. remove dead
    vector<Track> alive;
    for (auto& t : tracks) {
        if (t.missed <= max_missed)
            alive.push_back(t);
    }
    tracks.swap(alive);

    return output;
}

float SORT::iou(const TrackedDetection& a, const Detection& b) {
    float x1 = max(a.x1, b.x1);
    float y1 = max(a.y1, b.y1);
    float x2 = min(a.x2, b.x2);
    float y2 = min(a.y2, b.y2);

    float inter = max(0.f, x2 - x1) * max(0.f, y2 - y1);
    float area_a = (a.x2 - a.x1) * (a.y2 - a.y1);
    float area_b = (b.x2 - b.x1) * (b.y2 - b.y1);

    return inter / (area_a + area_b - inter + 1e-6f);
}
