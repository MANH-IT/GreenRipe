@echo off
setlocal enabledelayedexpansion

set "MSYS=C:\msys64\mingw64"
set "PATH=%MSYS%\bin;%PATH%"
cd /d "%~dp0"

if not exist "build\detect.exe" (
    echo [1/2] Cau hinh va build...
    if exist "%MSYS%\lib\cmake\opencv5" (
        set "OCV_DIR=%MSYS%/lib/cmake/opencv5"
    ) else (
        set "OCV_DIR=%MSYS%/lib/cmake/opencv4"
    )
    cmake -S . -B build -G "MinGW Makefiles" -DOpenCV_DIR=!OCV_DIR!
    if errorlevel 1 goto :error
    cmake --build build -j
    if errorlevel 1 goto :error
)

if not exist "output" mkdir "output"

set "FAILED=0"
for %%F in (images\*.jpg images\*.jpeg images\*.png images\*.webp images\*.bmp) do (
    if exist "%%~fF" (
        echo.
        echo === %%~nxF ===
        build\detect.exe "%%~fF" --no-gui ^
            -o "output\%%~nF_ketqua.png" ^
            --mask-chin "output\%%~nF_mask_chin.png" ^
            --mask-xanh "output\%%~nF_mask_xanh.png"
        if errorlevel 1 set "FAILED=1"
    )
)

echo.
if "%FAILED%"=="1" (
    echo Mot so anh xu ly that bai.
    exit /b 1
)
echo Da xu ly tat ca anh trong images\ va luu vao output\.
exit /b 0

:error
echo.
echo Build that bai. Kiem tra MSYS2/OpenCV da cai chua.
exit /b 1
