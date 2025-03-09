#ifndef TRACKBAR_H
#define TRACKBAR_H

#include <opencv.hpp>

#include <memory>
// fmt includes
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <fmt/args.h>

#include "params/tb_params.h"
#include "params/tb_callback.h"

// TODO: meta-programming, compile based on IS_DEBUGGING definition 

// should adhere to ELM architecture as much as possible
namespace tb {

// NOTE: maybe remove the shared_ptr type and use bool&?
std::shared_ptr<bool> init(const bool init_state, const tb::p& parameters = tb::p());

void update();

void view();

// NOTE: not a good function name
void act();

void show(const cv::Mat& image);

void show(const std::vector<cv::Mat>& images, bool combine_horizontally = true);

void show(const std::vector<std::vector<cv::Mat>>& images);

}

#endif // !TRACKBAR_H
