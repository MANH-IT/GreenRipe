#pragma once

// OpenCV 5 tach cac ham hinh hoc (contourArea, arcLength, approxPolyDP,
// boundingRect, minAreaRect, moments...) ra header rieng geometry/2d.hpp.
// OpenCV 4 lai dat chung trong imgproc.hpp. Include co dieu kien de
// tuong thich ca hai phien ban.

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#if defined(__has_include)
#  if __has_include(<opencv2/geometry/2d.hpp>)
#    include <opencv2/geometry/2d.hpp>
#  endif
#endif
