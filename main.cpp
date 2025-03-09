#include <opencv.hpp>

#include "opencv2/imgcodecs.hpp"
#include "src/trackbar.h"
// #include "src/tb_internal.h" // INFO: not essential
#include "src/util/constants.h"

// temporary
#include <numeric>

using tb::p;

void func()
{
    int g_size = 3;
    float g_sigma = 7.5f;
    int sobel_size = 3,
        sobel_dx = 1;
    double sobel_scale = 1.5;
    bool ex_condition = false;

    p pp("process_name", (
        p("gauss: size", {&g_size, 75, 3, tb::cb_odd}),
        p("gauss: sigma", {&g_sigma, 10}),
        p("sobel", (
            p("size", {&sobel_size, 15, 3, tb::cb_odd}),
            p("dx", {&sobel_dx, 15, 1, tb::cb_less_than(&sobel_size)}),
            p("scale", {&sobel_scale, 5, -5})
        )),
        p("ex_header_1: kernel", (
            p("flip kernel?", {&ex_condition}),
            p("ex_header_2", (
                p("are you sure?", {&ex_condition}),
                p("ex_header_3", (
                    p("ok", {&ex_condition})
                ))
            ))
        ))
    ));

    p pp_shorthand{
        {"gauss: size", {&g_size, 75, 3, tb::cb_odd}},
        {"gauss: sigma", {&g_sigma, 10}},
        {"sobel: scale", {&sobel_scale, 5, -5}},
        {"flip kernel?", {&ex_condition}},
    };

    // TODO: change shared_ptr<bool> to bool&
    // auto debugging = tb::init(true);
    std::shared_ptr<bool> debugging = tb::init(true, pp);

    do {
        tb::update();

        constexpr int color_bg = 20,
                      color_fg = 40,
                      thickness = 3;

        cv::Mat x(cv::Size(800, 800), CV_8UC1, cv::Scalar(color_bg));
        for (int i = 0; i < thickness; ++i) {
            const int j = i - (thickness/2);
            x.diag(j).setTo(cv::Scalar(color_fg));
        }
        cv::Mat xr;
        cv::flip(x, xr, 1);
        x |= xr;
        tb::show(x);

        cv::Mat x_derived;
        cv::Sobel(x, x_derived, -1, sobel_dx, 1, sobel_size, sobel_scale);
        tb::show(x_derived);

        cv::Mat pattern(cv::Size(1024, 1024), CV_8UC1, cv::Scalar(color_bg));
        for (int i = 0; i < pattern.rows; i += thickness*3) {
            pattern.rowRange(i, std::min(i+thickness, pattern.rows)).setTo(cv::Scalar(color_fg));
        }
        tb::show(pattern);

        cv::Mat flag(cv::Size(512, 512), CV_8UC3, cns::WHITE);
        cv::circle(flag, cv::Point(flag.cols/2, flag.rows/2), 128, cns::RED, cv::LineTypes::FILLED);
        tb::show(flag);

        // temporary
        std::vector<uchar> debug_vec(256);
        std::iota(debug_vec.begin(), debug_vec.end(), uchar{0});
        cv::Mat debug_mat{cv::Size(debug_vec.size(), 1), CV_8UC1, debug_vec.data()};
        debug_mat = cv::repeat(debug_mat, debug_vec.size(), 1);
        for (size_t i = 0; i < debug_mat.rows; i += 2)
            cv::flip(debug_mat.row(i), debug_mat.row(i), 1);
        tb::show(debug_mat);

        cv::Mat lenna = cv::imread("img/lenna.png", cv::ImreadModes::IMREAD_COLOR);
        tb::show(lenna);

        tb::view();
    } while (*debugging);
}

int main()
{
    func();

    return 0;
}

