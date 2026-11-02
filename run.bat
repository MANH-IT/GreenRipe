@echo off
setlocal

set "MSYS=C:\msys64\mingw64"
set "PATH=%MSYS%\bin;%PATH%"

REM Backend Qt6 co the khong hien cua so tren mot so ban Windows;
REM chon Win32 de cua so hien binh thuong.
set "OPENCV_UI_BACKEND=WIN32"

cd /d "%~dp0"

if exist "%MSYS%\lib\cmake\opencv5" (
    set "OCV_DIR=%MSYS%/lib/cmake/opencv5"
) else (
    set "OCV_DIR=%MSYS%/lib/cmake/opencv4"
)

if not exist build (
    echo [1/2] Cau hinh CMake...
    cmake -S . -B build -G "MinGW Makefiles" -DOpenCV_DIR=%OCV_DIR%
    if errorlevel 1 goto :error
)

echo [2/2] Build...
cmake --build build -j
if errorlevel 1 goto :error

REM Khong truyen tham so -> dung anh mau images\chuoixanh_chin2.jpg
build\detect.exe %*
exit /b %errorlevel%

:error
echo.
echo Build that bai. Kiem tra MSYS2/OpenCV da cai chua.
exit /b 1
