#pragma once

#include <opencv2/core.hpp>

namespace vision {

// Nguong phan loai truc tiep tren kenh mau BGR (khong dung HSV).
//
// Y tuong: sau khi loai nen, mau vang (chin) co R gan bang G nen G/R nho,
// con mau xanh co G lon hon R ro ret nen G/R lon. Tren anh mau, hai nhom
// tach nhau quanh G/R ~ 1.06.
//
// Voi moi pixel mau:
//   - bo qua pixel qua toi hoac gan nhu khong co mau (nen trang)
//   - bo qua pixel co nhieu xanh duong (B qua cao so voi G)
//   - G/R >= greenRatio  => qua xanh
//   - con lai            => qua chin (vang)
struct RgbThresholds {
    double greenRatio = 1.06;    // G/R >= nguong => qua xanh
    double maxBlueRatio = 0.75;  // B/G > nguong => loai pixel
    int minBrightness = 50;      // max(R,G,B) phai >= (loai vung qua toi)
    int minChroma = 25;          // max(R,G,B) - min(R,G,B) phai >= (loai nen xam/trang)
};

// Hai mask nhi phan cung kich thuoc voi anh dau vao.
struct RipenessMasks {
    cv::Mat ripe;   // CV_8UC1: 255 = qua chin (vang)
    cv::Mat green;  // CV_8UC1: 255 = qua xanh
};

// Tach mask qua chin / qua xanh tu anh BGR, co loc nhieu morphology.
//
// Tham so:
//   - bgr: anh dau vao 3 kenh (B, G, R), khong can chuyen sang HSV.
//   - thresholds: bo nguong phan loai, mac dinh la RgbThresholds{}.
//   - morphSize: kich thuoc kernel ELLIPSE; > 1 thi open (bo dom nho)
//     roi close (lap lo hong / noi vung bi dut).
//
// Tra ve 2 mask CV_8UC1 cung kich thuoc anh. Neu anh rong hoac khong phai
// 3 kenh thi tra ve 2 mask rong.
RipenessMasks segmentByRipeness(const cv::Mat& bgr,
                                const RgbThresholds& thresholds = {},
                                int morphSize = 7);

}  // namespace vision
