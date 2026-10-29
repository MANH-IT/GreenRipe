#include <opencv2/opencv.hpp>

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "vision/fruit_detector.hpp"
#include "vision/ripeness.hpp"

using namespace cv;
using namespace std;
using namespace vision;

// Anh mau di kem repo, chay duoc ngay sau khi clone.
static const char* kDefaultImage = "images/chuoixanh_chin2.jpg";

// Gia tri mac dinh cua bo phan loai BGR.
static constexpr int kDefaultGreenRatio = 106;   // 1.06
static constexpr int kDefaultBlueRatio = 75;     // 0.75
static constexpr int kDefaultBrightness = 50;
static constexpr int kDefaultChroma = 25;

// ---- Trackbar chinh nguong mau BGR (khong dung HSV) ----
static int tbGreenRatio = kDefaultGreenRatio;
static int tbBlueRatio = kDefaultBlueRatio;
static int tbBrightness = kDefaultBrightness;
static int tbChroma = kDefaultChroma;

static const Scalar kRipeColor(0, 255, 255);   // vang (BGR)
static const Scalar kGreenColor(0, 255, 0);    // xanh la (BGR)

// Cac tham so lay tu dong lenh.
struct Params {
    string imagePath = kDefaultImage;      // anh dau vao
    string outPath = "ketqua_chuoi.png";   // anh ket qua
    string ripeMaskPath = "mask_chin.png"; // mask qua chin
    string greenMaskPath = "mask_xanh.png";// mask qua xanh
    int minArea = 3000;                    // dien tich contour toi thieu
    bool noGui = false;                    // true: khong mo cua so
    bool help = false;                     // true: chi in huong dan
};

// In huong dan su dung ra man hinh.
static void printUsage(const char* programName) {
    const string name = (programName && *programName) ? programName : "detect";
    cout << "Dung: " << name << " [anh] [tuy chon]\n\n"
         << "Phan loai qua chin (vang) / qua xanh trong ANH bang kenh mau BGR,\n"
         << "khong dung HSV, khong dung camera.\n\n"
         << "Vi du:\n"
         << "  " << name << "\n"
         << "  " << name << " images/chuoixanh_chin2.jpg\n"
         << "  " << name << " images/chuoixanh_chin2.jpg --no-gui\n"
         << "  " << name << " -o ketqua.png --mask-chin chin.png --mask-xanh xanh.png\n\n"
         << "Tuy chon:\n"
         << "  -o, --out <path>        anh ket qua (mac dinh " << Params().outPath << ")\n"
         << "      --mask-chin <path>  mask qua chin (mac dinh " << Params().ripeMaskPath << ")\n"
         << "      --mask-xanh <path>  mask qua xanh (mac dinh " << Params().greenMaskPath << ")\n"
         << "  -m, --min-area <n>      dien tich contour toi thieu (mac dinh 3000)\n"
         << "      --no-gui            khong mo cua so, luu anh roi thoat\n"
         << "  -h, --help              hien thi huong dan nay\n";
}

// Chuyen chuoi sang so nguyen; tra false neu con ky tu la hoac tran so.
static bool parseInt(const string& text, int& value) {
    try {
        size_t used = 0;
        const int parsed = stoi(text, &used);
        if (used != text.size()) return false;
        value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

// Doc tham so dong lenh.
// Tra ve false va gan `error` neu tham so sai / thieu gia tri.
static bool parseParams(int argc, char** argv, Params& out, string& error) {
    // Lay gia tri ngay sau mot tuy chon, vi du: --out ketqua.png
    auto needValue = [&](int& i, const char* option, string& value) -> bool {
        if (i + 1 >= argc) {
            error = string("Thieu gia tri cho tham so ") + option;
            return false;
        }
        value = argv[++i];
        return true;
    };

    for (int i = 1; i < argc; ++i) {
        const string arg = argv[i] ? argv[i] : "";

        if (arg == "-h" || arg == "--help") {
            out.help = true;
            return true;
        }
        if (arg == "-o" || arg == "--out") {
            if (!needValue(i, "--out", out.outPath)) return false;
            continue;
        }
        if (arg == "--mask-chin") {
            if (!needValue(i, "--mask-chin", out.ripeMaskPath)) return false;
            continue;
        }
        if (arg == "--mask-xanh") {
            if (!needValue(i, "--mask-xanh", out.greenMaskPath)) return false;
            continue;
        }
        if (arg == "-m" || arg == "--min-area") {
            string value;
            if (!needValue(i, "--min-area", value)) return false;
            if (!parseInt(value, out.minArea) || out.minArea < 0) {
                error = "Gia tri --min-area phai la so khong am";
                return false;
            }
            continue;
        }
        if (arg == "--no-gui" || arg == "--nogui") {
            out.noGui = true;
            continue;
        }
        // Bat dau bang '-' nhung khong khop tuy chon nao => tham so la.
        if (!arg.empty() && arg[0] == '-') {
            error = "Tham so khong hop le: " + arg;
            return false;
        }

        // Tham so vi tri dau tien la duong dan anh.
        // Chi cho phep mot anh; anh thu hai la loi.
        if (out.imagePath == kDefaultImage) {
            out.imagePath = arg;
        } else {
            error = "Khong hieu tham so vi tri: " + arg;
            return false;
        }
    }
    return true;
}

// Tao thu muc cha cua file neu chua ton tai (de imwrite khong that bai).
static void ensureParentDir(const string& path) {
    std::error_code ec;
    auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) std::filesystem::create_directories(parent, ec);
}

// Chuyen gia tri trackbar (so nguyen) sang bo nguong thuc.
// Hai ti le duoc luu dang phan tram: 106 -> 1.06, 75 -> 0.75.
static RgbThresholds thresholdsFromTrackbars() {
    RgbThresholds t;
    t.greenRatio = tbGreenRatio / 100.0;
    t.maxBlueRatio = tbBlueRatio / 100.0;
    t.minBrightness = tbBrightness;
    t.minChroma = tbChroma;
    return t;
}

// Tao anh ket qua: to mau vung qua + ve khung + dong thong ke.
static Mat buildResult(const Mat& bgr,
                       const RipenessMasks& masks,
                       const vector<FruitRegion>& regions) {
    Mat result = bgr.clone();

    // To mau mo len cac vung da nhan dien.
    // tint = ban sao cua anh, chi doi mau tai vi tri mask = 255.
    Mat tint = bgr.clone();
    tint.setTo(kRipeColor, masks.ripe);
    tint.setTo(kGreenColor, masks.green);

    // Tron 72% anh goc + 28% anh to mau. Nhung pixel khong co mask thi
    // tint trung voi bgr nen ket qua khong doi.
    addWeighted(bgr, 0.72, tint, 0.28, 0.0, result);

    drawFruits(result, regions);

    // Dem so vung theo tung loai de hien thi.
    int ripeCount = 0;
    int greenCount = 0;
    for (const auto& r : regions) {
        if (r.ripeness == Ripeness::Ripe) ++ripeCount;
        else ++greenCount;
    }

    char buf[256];
    snprintf(buf, sizeof(buf), "Chin (vang): %d vung   Xanh: %d vung",
             ripeCount, greenCount);

    // Ve vien den ben duoi roi chu trang ben tren cho de doc.
    putText(result, buf, {10, 30}, FONT_HERSHEY_SIMPLEX, 0.8,
            Scalar(0, 0, 0), 5);
    putText(result, buf, {10, 30}, FONT_HERSHEY_SIMPLEX, 0.8,
            Scalar(255, 255, 255), 2);
    return result;
}

// Gop 2 mask thanh anh mau de hien thi trong cua so chinh nguong:
// vang = qua chin, xanh la = qua xanh.
static Mat colorizeMasks(const RipenessMasks& masks) {
    Mat combined = Mat::zeros(masks.ripe.size(), CV_8UC3);
    combined.setTo(kRipeColor, masks.ripe);
    combined.setTo(kGreenColor, masks.green);
    return combined;
}

// Luu anh ket qua + 2 mask ra dia. Tra false neu bat ky file nao loi.
static bool saveOutputs(const Params& p,
                        const Mat& result,
                        const RipenessMasks& masks) {
    // Dam bao thu muc cha ton tai truoc khi ghi.
    ensureParentDir(p.outPath);
    ensureParentDir(p.ripeMaskPath);
    ensureParentDir(p.greenMaskPath);

    if (!imwrite(p.outPath, result)) {
        cerr << "Khong luu duoc " << p.outPath << "\n";
        return false;
    }
    if (!imwrite(p.ripeMaskPath, masks.ripe)) {
        cerr << "Khong luu duoc " << p.ripeMaskPath << "\n";
        return false;
    }
    if (!imwrite(p.greenMaskPath, masks.green)) {
        cerr << "Khong luu duoc " << p.greenMaskPath << "\n";
        return false;
    }
    cout << "Da luu " << p.outPath << ", " << p.ripeMaskPath
         << " va " << p.greenMaskPath << "\n";
    return true;
}

// In so vung va tong dien tich theo tung loai (dung cho che do --no-gui).
static void printCounts(const vector<FruitRegion>& regions) {
    int ripeCount = 0;
    int greenCount = 0;
    double ripeArea = 0.0;
    double greenArea = 0.0;

    for (const auto& r : regions) {
        if (r.ripeness == Ripeness::Ripe) {
            ++ripeCount;
            ripeArea += r.area;
        } else {
            ++greenCount;
            greenArea += r.area;
        }
    }

    cout << "Qua chin (vang): " << ripeCount << " vung"
         << ", dien tich " << static_cast<long long>(ripeArea) << "\n";
    cout << "Qua xanh: " << greenCount << " vung"
         << ", dien tich " << static_cast<long long>(greenArea) << "\n";
}

int main(int argc, char** argv) {
    // 1) Doc tham so dong lenh.
    Params params;
    string error;
    if (!parseParams(argc, argv, params, error)) {
        cerr << error << "\n\n";
        printUsage(argv[0]);
        return 2;
    }
    if (params.help) {
        printUsage(argv[0]);
        return 0;
    }

    // 2) Doc anh BGR. Khong goi cvtColor sang HSV.
    Mat bgr = imread(params.imagePath, IMREAD_COLOR);
    if (bgr.empty()) {
        cerr << "Khong doc duoc anh: " << params.imagePath << "\n";
        return 1;
    }

    // In thong tin anh va bo nguong mac dinh.
    cout << "Anh: " << params.imagePath
         << " (" << bgr.cols << "x" << bgr.rows << ")\n";
    cout << "Phan loai bang BGR: G/R >= " << RgbThresholds().greenRatio
         << " => xanh, B/G > " << RgbThresholds().maxBlueRatio
         << " => bo qua, do sang >= " << RgbThresholds().minBrightness
         << ", chenh mau >= " << RgbThresholds().minChroma << "\n";

    // 3) Che do --no-gui: xu ly mot lan, luu file roi thoat.
    if (params.noGui) {
        const RgbThresholds thresholds;
        const RipenessMasks masks = segmentByRipeness(bgr, thresholds);
        const vector<FruitRegion> regions =
            detectFruits(bgr, thresholds, params.minArea);
        Mat result = buildResult(bgr, masks, regions);

        printCounts(regions);
        return saveOutputs(params, result, masks) ? 0 : 1;
    }

    // 4) Che do GUI: mo cac cua so va tao trackbar.
    namedWindow("Qua chin / Qua xanh", WINDOW_AUTOSIZE);
    namedWindow("Mask qua chin", WINDOW_AUTOSIZE);
    namedWindow("Mask qua xanh", WINDOW_AUTOSIZE);
    namedWindow("Chinh nguong mau", WINDOW_AUTOSIZE);
    createTrackbar("G/R xanh", "Chinh nguong mau", &tbGreenRatio, 200);
    createTrackbar("B/G toi da", "Chinh nguong mau", &tbBlueRatio, 100);
    createTrackbar("Sang toi thieu", "Chinh nguong mau", &tbBrightness, 255);
    createTrackbar("Chenh mau", "Chinh nguong mau", &tbChroma, 150);

    int minArea = params.minArea;

    // 5) Vong lap hien thi: moi lan trackbar thay doi, xu ly lai tu dau.
    while (true) {
        // Lay bo nguong hien tai tu cac trackbar.
        const RgbThresholds thresholds = thresholdsFromTrackbars();

        // Tach mask va tim vung qua.
        const RipenessMasks masks = segmentByRipeness(bgr, thresholds);
        const vector<FruitRegion> regions =
            detectFruits(bgr, thresholds, minArea);
        Mat result = buildResult(bgr, masks, regions);

        // Dong huong dan o cuoi anh.
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "minArea: %d  (m: doi)  (r: reset nguong)", minArea);
        putText(result, buf, {10, bgr.rows - 15},
                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255), 1);
        putText(result, "s: luu  q: thoat", {10, bgr.rows - 35},
                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255), 1);

        // Hien thi: anh ket qua + 2 mask + anh mau cua 2 mask.
        imshow("Qua chin / Qua xanh", result);
        imshow("Mask qua chin", masks.ripe);
        imshow("Mask qua xanh", masks.green);
        imshow("Chinh nguong mau", colorizeMasks(masks));

        // Cho phim, cho toi da 20 ms de vong lap chay lien tuc.
        const int key = waitKey(20) & 0xFF;

        // q hoac ESC: thoat.
        if (key == 27 || key == 'q') break;

        if (key == 's') {
            // s: luu anh ket qua va mask.
            saveOutputs(params, result, masks);
        } else if (key == 'm') {
            // m: doi nhanh nguong dien tich giua 3000 va 10000.
            minArea = (minArea >= 10000) ? 3000 : 10000;
            cout << "Nguong dien tich: " << minArea << "\n";
        } else if (key == 'r') {
            // r: dua 4 trackbar ve gia tri mac dinh.
            setTrackbarPos("G/R xanh", "Chinh nguong mau",
                           tbGreenRatio = kDefaultGreenRatio);
            setTrackbarPos("B/G toi da", "Chinh nguong mau",
                           tbBlueRatio = kDefaultBlueRatio);
            setTrackbarPos("Sang toi thieu", "Chinh nguong mau",
                           tbBrightness = kDefaultBrightness);
            setTrackbarPos("Chenh mau", "Chinh nguong mau",
                           tbChroma = kDefaultChroma);
            cout << "Da reset nguong BGR ve mac dinh\n";
        }
    }

    return 0;
}
