#include "vision/ripeness.hpp"

#include <algorithm>

#include <opencv2/imgproc.hpp>

namespace vision {
namespace {

// Lam sach mask nhi phan:
//   - OPEN  : xoa cac dom nho / nhieu roi rac.
//   - CLOSE : lap lo hong ben trong va noi cac vung bi dut nhe.
void cleanMask(cv::Mat& mask, int morphSize) {
    if (morphSize <= 1 || mask.empty()) return;

    // Kernel ELLIPSE cho vien tron, giong hinh dang qua tu nhien hon hinh vuong.
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_ELLIPSE, cv::Size(morphSize, morphSize));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
}

}  // namespace

RipenessMasks segmentByRipeness(const cv::Mat& bgr,
                                const RgbThresholds& thresholds,
                                int morphSize) {
    RipenessMasks masks;

    // Chi xu ly anh BGR 3 kenh; anh rong hoac sai dinh dang => tra mask rong.
    if (bgr.empty() || bgr.channels() != 3) return masks;

    // Khoi tao 2 mask den (0 = khong thuoc vung, 255 = thuoc vung).
    masks.ripe = cv::Mat::zeros(bgr.size(), CV_8UC1);
    masks.green = cv::Mat::zeros(bgr.size(), CV_8UC1);

    // Duyet tung pixel. Dung con tro theo hang de tranh goi at<> nhieu lan.
    for (int y = 0; y < bgr.rows; ++y) {
        const uchar* row = bgr.ptr<uchar>(y);
        uchar* ripeRow = masks.ripe.ptr<uchar>(y);
        uchar* greenRow = masks.green.ptr<uchar>(y);

        for (int x = 0; x < bgr.cols; ++x) {
            // Anh BGR trong OpenCV luu theo thu tu B, G, R.
            const int b = row[x * 3 + 0];
            const int g = row[x * 3 + 1];
            const int r = row[x * 3 + 2];

            // Do sang = kenh lon nhat; do chenh mau = max - min.
            const int maxChannel = std::max({r, g, b});
            const int minChannel = std::min({r, g, b});

            // 1) Loai vung qua toi (khong du anh sang de phan loai).
            if (maxChannel < thresholds.minBrightness) continue;

            // 2) Loai nen trang / xam: ba kenh gan bang nhau nen chenh mau thap.
            if (maxChannel - minChannel < thresholds.minChroma) continue;

            // 3) Loai vung co anh xanh duong manh (B qua cao so voi G).
            if (b > thresholds.maxBlueRatio * g) continue;

            // 4) Phan loai con lai dua vao ti le G/R:
            //    G/R lon => nhieu xanh la => qua xanh;
            //    G/R nho => vang (R gan G) => qua chin.
            if (g >= thresholds.greenRatio * r)
                greenRow[x] = 255;
            else
                ripeRow[x] = 255;
        }
    }

    cleanMask(masks.ripe, morphSize);
    cleanMask(masks.green, morphSize);
    return masks;
}

}  // namespace vision
