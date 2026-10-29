# Bài tập OpenCV: Nhận diện quả chín / quả xanh trong ảnh (BGR)

Chương trình **chỉ xử lý ảnh**, không dùng camera và **không dùng HSV**.
Mặc định đọc ảnh:

```
D:\OBS\chuoixanh_chin2.jpg
```

Mỗi pixel được phân loại trực tiếp từ 3 kênh **B, G, R**:

| Điều kiện | Kết quả |
|-----------|---------|
| `max(R,G,B) < 50` | bỏ qua (quá tối) |
| `max(R,G,B) - min(R,G,B) < 25` | bỏ qua (nền trắng / gần xám) |
| `B > 0.75 * G` | bỏ qua (ánh xanh dương) |
| `G >= 1.06 * R` | **quả xanh** |
| còn lại | **quả chín (vàng)** |

Sau đó tìm contour trên từng mask, lọc theo diện tích và khoanh vùng quả.

## 1. Tính năng

- Đọc ảnh mặc định `D:\OBS\chuoixanh_chin2.jpg` hoặc ảnh truyền vào.
- Phân loại **quả chín (vàng)** và **quả xanh** bằng tỉ lệ kênh BGR, không HSV.
- Lọc nhiễu + nối vùng bằng morphology (open + close).
- Tìm vùng bằng `cv::findContours`, lọc theo diện tích tối thiểu (mặc định 3000).
- Tô màu: vàng cho quả chín, xanh lá cho quả xanh; vẽ khung, tâm, nhãn
  `Chin #n A=...` / `Xanh #n A=...`.
- Cửa sổ **"Chỉnh ngưỡng màu"** với 4 trackbar BGR để chỉnh trực tiếp.
- Lưu ảnh kết quả + 2 mask bằng phím `s` hoặc chế độ `--no-gui`.
- Test bằng CTest trên ảnh BGR tổng hợp, không cần ảnh thật.

## 2. Cấu trúc thư mục

```
BaiTap_ChuoiXanhChin/
├── CMakeLists.txt
├── run.bat                       # build + chạy trên Windows/MSYS2
├── include/vision/
│   ├── ripeness.hpp              # RgbThresholds + segmentByRipeness
│   ├── fruit_detector.hpp        # phát hiện + vẽ vùng quả
│   └── opencv_compat.hpp         # tương thích OpenCV 4 / 5
├── src/
│   ├── ripeness.cpp
│   ├── fruit_detector.cpp
│   └── main.cpp                  # đọc ảnh, trackbar, lưu kết quả
└── tests/
    ├── test_util.hpp
    ├── test_ripeness.cpp
    └── test_fruit_detector.cpp
```

## 3. Yêu cầu

- CMake >= 3.16, trình biên dịch C++17
- OpenCV >= 4.x (`find_package(OpenCV)`)

Windows (MSYS2 MinGW64):

```bat
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-opencv
```

## 4. Build và chạy

Cách nhanh trên Windows:

```bat
run.bat
run.bat --no-gui
```

Hoặc build thủ công:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

```bat
build\detect.exe                                  REM ảnh mặc định
build\detect.exe "D:\OBS\chuoixanh_chin2.jpg"
build\detect.exe --no-gui
build\detect.exe --min-area 5000
build\detect.exe -h
```

## 5. Tham số dòng lệnh

| Tham số | Ý nghĩa |
|---------|----------|
| `<ảnh>` | Ảnh đầu vào (mặc định `D:\OBS\chuoixanh_chin2.jpg`) |
| `-o`, `--out <path>` | Ảnh kết quả (mặc định `ketqua_chuoi.png`) |
| `--mask-chin <path>` | Mask quả chín (mặc định `mask_chin.png`) |
| `--mask-xanh <path>` | Mask quả xanh (mặc định `mask_xanh.png`) |
| `-m`, `--min-area <n>` | Diện tích contour tối thiểu (mặc định 3000) |
| `--no-gui` | Không mở cửa sổ, lưu ảnh rồi thoát |
| `-h`, `--help` | Hiển thị hướng dẫn |

## 6. Phím điều khiển (chế độ GUI)

| Phím | Chức năng |
|------|-----------|
| `q` / `ESC` | Thoát |
| `s` | Lưu ảnh kết quả + 2 mask |
| `m` | Đổi ngưỡng diện tích (3000 / 10000) |
| `r` | Reset 4 ngưỡng BGR về mặc định |

Trackbar trong cửa sổ **"Chỉnh ngưỡng màu"**:

| Trackbar | Mặc định | Ý nghĩa |
|----------|----------|----------|
| `G/R xanh` | 106 | `G/R >= 1.06` → quả xanh |
| `B/G tối đa` | 75 | `B/G > 0.75` → bỏ qua |
| `Sáng tối thiểu` | 50 | `max(R,G,B) >= 50` |
| `Chênh màu` | 25 | `max-min >= 25` |

## 7. Cách hoạt động

```
ảnh BGR
  → đọc từng pixel (B, G, R)
  → loại pixel tối / nền trắng / ánh xanh dương
  → G/R >= 1.06 ? mask xanh : mask chín
  → morphologyEx(open) + morphologyEx(close)
  → findContours → contourArea >= minArea
  → boundingRect + moments
  → tô màu, vẽ khung, lưu kết quả
```

Với `chuoixanh_chin2.jpg` (960×678), cách phân loại này cho:

- **1 vùng quả chín** (nửa trái ảnh, diện tích ~153.900 pixel).
- **1 vùng quả xanh** (nửa phải ảnh, diện tích ~114.400 pixel).

Không có bước `cvtColor` sang HSV ở bất kỳ đâu.

Lưu ý: chương trình nhận diện **vùng quả chín / vùng quả xanh**, không đếm từng
quả riêng lẻ bên trong chùm.

## 8. Test

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

- `test_ripeness`: kiểm tra phân loại pixel vàng/xanh, loại nền trắng, vật xanh
  dương, pixel tối, nhiễu 1 pixel và thay đổi ngưỡng G/R.
- `test_fruit_detector`: kiểm tra phát hiện 1 vùng chín + 1 vùng xanh, lọc
  `minArea`, sắp xếp và vẽ kết quả.
