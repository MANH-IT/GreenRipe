#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "test_util.hpp"
#include "vision/ripeness.hpp"

using namespace vision;

int main() {
    // Kiem tra bo nguong mac dinh dung voi tai lieu / README.
    const RgbThresholds defaults;
    testx::check(defaults.greenRatio == 1.06, "ty le G/R mac dinh = 1.06");
    testx::check(defaults.maxBlueRatio == 0.75, "ty le B/G toi da = 0.75");
    testx::check(defaults.minBrightness == 50, "do sang toi thieu = 50");
    testx::check(defaults.minChroma == 25, "chenh mau toi thieu = 25");

    // Anh BGR tong hop: nen trang + vung vang + vung xanh + vung xanh duong.
    // Gia tri Scalar trong OpenCV la (B, G, R).
    cv::Mat bgr(300, 400, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::rectangle(bgr, cv::Point(50, 80), cv::Point(180, 220),
                  cv::Scalar(0, 220, 240), cv::FILLED);    // vang (chin)
    cv::rectangle(bgr, cv::Point(220, 80), cv::Point(350, 220),
                  cv::Scalar(50, 200, 130), cv::FILLED);   // xanh
    cv::rectangle(bgr, cv::Point(20, 250), cv::Point(80, 290),
                  cv::Scalar(255, 0, 0), cv::FILLED);      // xanh duong

    // morphSize = 0 de tat morphology, kiem tra dung tung pixel phan loai.
    RipenessMasks masks = segmentByRipeness(bgr, defaults, 0);
    testx::check(masks.ripe.at<uchar>(150, 100) == 255,
                 "vung vang duoc phan loai la qua chin");
    testx::check(masks.green.at<uchar>(150, 100) == 0,
                 "vung vang khong bi nhan la qua xanh");
    testx::check(masks.green.at<uchar>(150, 280) == 255,
                 "vung xanh duoc phan loai la qua xanh");
    testx::check(masks.ripe.at<uchar>(150, 280) == 0,
                 "vung xanh khong bi nhan la qua chin");
    testx::check(masks.ripe.at<uchar>(20, 20) == 0 &&
                 masks.green.at<uchar>(20, 20) == 0,
                 "nen trang bi loai (chenh mau thap)");
    testx::check(masks.ripe.at<uchar>(270, 50) == 0 &&
                 masks.green.at<uchar>(270, 50) == 0,
                 "vat xanh duong bi loai (B/G qua cao)");

    // Pixel toi cung bi loai.
    cv::Mat dark = bgr.clone();
    dark.at<cv::Vec3b>(290, 390) = cv::Vec3b(10, 10, 10);
    RipenessMasks darkMasks = segmentByRipeness(dark, defaults, 0);
    testx::check(darkMasks.ripe.at<uchar>(290, 390) == 0 &&
                 darkMasks.green.at<uchar>(290, 390) == 0,
                 "pixel qua toi bi loai");

    // Morphology (open) phai xoa duoc mot dom nhieu chi 1 pixel.
    cv::Mat noisy(100, 100, CV_8UC3, cv::Scalar(255, 255, 255));
    noisy.at<cv::Vec3b>(10, 10) = cv::Vec3b(0, 220, 240);
    RipenessMasks clean = segmentByRipeness(noisy, defaults, 7);
    testx::check(clean.ripe.at<uchar>(10, 10) == 0,
                 "morphology loai nhieu 1 pixel");

    // Doi nguong G/R lam thay doi phan loai cua cung mot mau.
    // Pixel (B=100, G=240, R=200) co G/R = 1.2.
    cv::Mat oneGreen(50, 50, CV_8UC3, cv::Scalar(100, 240, 200));  // G/R = 1.2
    RipenessMasks strict = segmentByRipeness(oneGreen, defaults, 0);
    testx::check(strict.green.at<uchar>(25, 25) == 255,
                 "mau G/R=1.2 la xanh voi nguong 1.06");

    RgbThresholds high = defaults;
    high.greenRatio = 1.3;
    RipenessMasks relaxed = segmentByRipeness(oneGreen, high, 0);
    testx::check(relaxed.ripe.at<uchar>(25, 25) == 255 &&
                 relaxed.green.at<uchar>(25, 25) == 0,
                 "mau G/R=1.2 thanh chin khi nguong xanh = 1.3");

    // Dau vao khong hop le phai tra mask rong, khong crash.
    RipenessMasks empty = segmentByRipeness(cv::Mat(), defaults, 0);
    testx::check(empty.ripe.empty() && empty.green.empty(),
                 "anh rong => mask rong");

    return testx::summary("ripeness");
}
