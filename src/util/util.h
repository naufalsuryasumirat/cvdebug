#ifndef CV_UTIL_H
#define CV_UTIL_H

#include <opencv.hpp>
#include <opencv2/core.hpp>

#include "constants.h" // IWYU pragma: export

namespace util {

bool is_same_image(const cv::Mat& src, const cv::Mat& cmp);

cv::Mat as_rgba(const cv::Mat& src);

}

#endif
