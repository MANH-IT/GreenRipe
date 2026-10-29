#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>

#include "vision/ripeness.hpp"

namespace vision {

// Loai qua theo do chin.
enum class Ripeness { Ripe, Green };

// Ten hien thi: "Chin" hoac "Xanh".
const char* ripenessName(Ripeness ripeness);

// Mot vung qua da nhan dien duoc.
struct FruitRegion {
    Ripeness ripeness = Ripeness::Ripe;  // vung nay la qua chin hay qua xanh
    double area = 0.0;                   // dien tich contour (pixel)
    cv::Rect box;                        // khung bao quanh vung qua
    cv::Point center;                    // tam vung qua (tu moments)
    cv::Scalar draw{0, 255, 255};        // mau BGR de ve (vang hoac xanh la)
};

// Tim vung qua chin va qua xanh tren anh BGR.
//
// Tham so:
//   - bgr: anh dau vao 3 kenh.
//   - thresholds: nguong phan loai BGR (xem ripeness.hpp).
//   - minArea: bo qua contour co dien tich nho hon gia tri nay.
//
// Tra ve danh sach vung, trong do cac vung qua chin dung truoc cac vung
// qua xanh; moi nhom duoc sap xep giam dan theo dien tich.
std::vector<FruitRegion> detectFruits(const cv::Mat& bgr,
                                      const RgbThresholds& thresholds,
                                      int minArea);

// Ve khung + nhan + tam cho tung vung qua len anh `image`.
// Nhan co dang "Chin #1 A=12345" hoac "Xanh #2 A=6789".
void drawFruits(cv::Mat& image, const std::vector<FruitRegion>& regions);

}  // namespace vision
