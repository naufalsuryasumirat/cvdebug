#include "util.h"

bool util::is_same_image(const cv::Mat& src, const cv::Mat& cmp)
{
    return src.size() == cmp.size()
        && src.channels() == cmp.channels()
        && cv::sum(src != cmp) == cv::Scalar(0, 0, 0, 0);
}

cv::Mat util::as_rgba(const cv::Mat &src)
{
    if (src.empty())
        throw std::logic_error("src is empty");

    cv::Mat res;
    if ((src.type() | CV_8U) == src.type() || (src.type() | CV_32F) == src.type()) {
        if (src.channels() == 1)
            cv::cvtColor(src, res, cv::ColorConversionCodes::COLOR_GRAY2RGBA);
        else
            cv::cvtColor(src, res, cv::ColorConversionCodes::COLOR_BGR2RGBA);
    } else throw std::logic_error("src is not of type {8U,32F}");

    return res;
}
