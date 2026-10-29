#include "vision/fruit_detector.hpp"

#include <algorithm>
#include <sstream>

#include "vision/opencv_compat.hpp"

namespace vision {
namespace {

// Mau ve tren anh BGR: vang cho qua chin, xanh la cho qua xanh.
const cv::Scalar kRipeColor(0, 255, 255);   // vang (BGR)
const cv::Scalar kGreenColor(0, 255, 0);    // xanh la (BGR)

// Chuyen mot mask nhi phan thanh danh sach vung qua.
// `ripeness` cho biet mask nay dai dien cho loai qua nao.
std::vector<FruitRegion> regionsFromMask(const cv::Mat& mask,
                                         Ripeness ripeness,
                                         int minArea) {
    std::vector<FruitRegion> regions;

    // RETR_EXTERNAL: chi lay duong bao ngoai, bo qua contour long ben trong.
    // CHAIN_APPROX_SIMPLE: chi giu cac dinh, giam so diem cua contour.
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        // Bo qua cac dom nhieu / vat the qua nho.
        const double area = cv::contourArea(contour);
        if (area < minArea) continue;

        FruitRegion region;
        region.ripeness = ripeness;
        region.draw = (ripeness == Ripeness::Ripe) ? kRipeColor : kGreenColor;
        region.area = area;
        region.box = cv::boundingRect(contour);

        // Tinh tam vung qua bang moments:
        //   center.x = m10 / m00, center.y = m01 / m00.
        // Neu m00 = 0 (contour suy bien) thi lay tam cua bounding box.
        const cv::Moments mo = cv::moments(contour);
        if (mo.m00 != 0.0)
            region.center = cv::Point(static_cast<int>(mo.m10 / mo.m00),
                                      static_cast<int>(mo.m01 / mo.m00));
        else
            region.center = cv::Point(region.box.x + region.box.width / 2,
                                      region.box.y + region.box.height / 2);

        regions.push_back(std::move(region));
    }
    return regions;
}

}  // namespace

const char* ripenessName(Ripeness ripeness) {
    return (ripeness == Ripeness::Ripe) ? "Chin" : "Xanh";
}

std::vector<FruitRegion> detectFruits(const cv::Mat& bgr,
                                      const RgbThresholds& thresholds,
                                      int minArea) {
    // Buoc 1: tach 2 mask nhi phan (chin / xanh) bang nguong BGR.
    const RipenessMasks masks = segmentByRipeness(bgr, thresholds);

    // Buoc 2: tim cac vung rieng biet tren tung mask.
    std::vector<FruitRegion> ripe = regionsFromMask(
        masks.ripe, Ripeness::Ripe, minArea);
    std::vector<FruitRegion> green = regionsFromMask(
        masks.green, Ripeness::Green, minArea);

    // Buoc 3: sap xep moi nhom giam dan theo dien tich, de vung lon nhat
    // duoc danh so #1.
    std::sort(ripe.begin(), ripe.end(),
              [](const FruitRegion& a, const FruitRegion& b) {
                  return a.area > b.area;
              });
    std::sort(green.begin(), green.end(),
              [](const FruitRegion& a, const FruitRegion& b) {
                  return a.area > b.area;
              });

    // Buoc 4: gop lai, qua chin dung truoc qua xanh.
    ripe.insert(ripe.end(), green.begin(), green.end());
    return ripe;
}

void drawFruits(cv::Mat& image, const std::vector<FruitRegion>& regions) {
    // Dem rieng de danh so "Chin #1", "Chin #2", "Xanh #1", ...
    int ripeIndex = 0;
    int greenIndex = 0;

    for (const auto& r : regions) {
        const int index = (r.ripeness == Ripeness::Ripe) ? ++ripeIndex : ++greenIndex;

        // Khung bao va tam vung qua.
        cv::rectangle(image, r.box, r.draw, 2);
        cv::circle(image, r.center, 5, r.draw, cv::FILLED);

        // Nhan: loai qua + so thu tu + dien tich.
        std::ostringstream os;
        os << ripenessName(r.ripeness) << " #" << index
           << " A=" << static_cast<int>(r.area);

        // Ve vien den truoc, chu mau sau de doc duoc tren moi nen.
        const cv::Point org(r.box.x, std::max(18, r.box.y - 8));
        cv::putText(image, os.str(), org, cv::FONT_HERSHEY_SIMPLEX,
                    0.6, cv::Scalar(0, 0, 0), 4);
        cv::putText(image, os.str(), org, cv::FONT_HERSHEY_SIMPLEX,
                    0.6, r.draw, 2);
    }
}

}  // namespace vision
