#include <algorithm>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "test_util.hpp"
#include "vision/fruit_detector.hpp"
#include "vision/ripeness.hpp"

using namespace vision;

namespace {

// Dem so vung thuoc mot loai (chin hoac xanh).
int countRipeness(const std::vector<FruitRegion>& regions, Ripeness ripeness) {
    int count = 0;
    for (const auto& r : regions)
        if (r.ripeness == ripeness) ++count;
    return count;
}

// Tim vung co dien tich lon nhat thuoc mot loai.
const FruitRegion* largestOf(const std::vector<FruitRegion>& regions,
                             Ripeness ripeness) {
    const FruitRegion* best = nullptr;
    for (const auto& r : regions) {
        if (r.ripeness != ripeness) continue;
        if (!best || r.area > best->area) best = &r;
    }
    return best;
}

}  // namespace

int main() {
    const RgbThresholds thresholds;

    // Nen trang + 1 vung chin (vang) + 1 vung xanh + 1 dom xanh nho.
    // Scalar(B, G, R): vang = (0,220,240), xanh = (50,200,130).
    cv::Mat bgr(300, 400, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::rectangle(bgr, cv::Point(50, 80), cv::Point(180, 220),
                  cv::Scalar(0, 220, 240), cv::FILLED);
    cv::rectangle(bgr, cv::Point(220, 80), cv::Point(350, 220),
                  cv::Scalar(50, 200, 130), cv::FILLED);
    cv::rectangle(bgr, cv::Point(20, 20), cv::Point(35, 35),
                  cv::Scalar(50, 200, 130), cv::FILLED);

    // minArea = 3000: chi giu vung chin va vung xanh lon, bo dom xanh nho.
    auto regions = detectFruits(bgr, thresholds, 3000);
    testx::check(regions.size() == 2, "minArea=3000 giu 2 vung lon");
    testx::check(countRipeness(regions, Ripeness::Ripe) == 1,
                 "co 1 vung qua chin");
    testx::check(countRipeness(regions, Ripeness::Green) == 1,
                 "co 1 vung qua xanh");
    testx::check(regions[0].ripeness == Ripeness::Ripe,
                 "vung chin duoc sap xep truoc vung xanh");

    const FruitRegion* ripe = largestOf(regions, Ripeness::Ripe);
    const FruitRegion* green = largestOf(regions, Ripeness::Green);
    testx::check(ripe && ripe->box.contains(cv::Point(115, 150)),
                 "box qua chin chua tam vung vang");
    testx::check(green && green->box.contains(cv::Point(285, 150)),
                 "box qua xanh chua tam vung xanh");
    testx::check(ripe && ripe->area > 15000.0, "dien tich vung chin lon");
    testx::check(green && green->area > 15000.0, "dien tich vung xanh lon");

    testx::check(std::string(ripenessName(Ripeness::Ripe)) == "Chin",
                 "ten qua chin");
    testx::check(std::string(ripenessName(Ripeness::Green)) == "Xanh",
                 "ten qua xanh");

    // Ha minArea xuong 100: dom xanh nho cung duoc tinh.
    auto all = detectFruits(bgr, thresholds, 100);
    testx::check(all.size() == 3, "minArea=100 bat ca dom xanh nho");
    testx::check(countRipeness(all, Ripeness::Green) == 2,
                 "co 2 vung xanh khi ha minArea");

    // minArea lon hon ca anh: khong con vung nao.
    auto none = detectFruits(bgr, thresholds, 1000000);
    testx::check(none.empty(), "minArea qua lon => khong co vung nao");

    // Ve thu de dam bao khong loi truy cap / khong crash.
    cv::Mat canvas = bgr.clone();
    drawFruits(canvas, regions);
    testx::check(!canvas.empty(), "ve vung qua khong loi");

    return testx::summary("fruit_detector");
}
