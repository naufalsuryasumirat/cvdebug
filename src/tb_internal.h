#ifndef TB_INTERNAL_H
#define TB_INTERNAL_H

#include "imgui/imgui.h"
#include "params/tb_params.h"

namespace tbd {

void bfsp(const tb::p& params);
void dfsp(const tb::p& params, const uint16_t depth = 0);

}

namespace ImGui {
// extended ImGui slider implementation
bool SliderIntW(
    const char* label, int* v, int v_min, int v_max,
    std::function<void(int*)> cb = nullptr,
    const char* format = "%d",
    ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp,
    int incrementer = 1);
bool SliderFloatW(
    const char* label, float* v, float v_min, float v_max,
    std::function<void(float*)> cb = nullptr,
    const char* format = "%.2f",
    ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp,
    float multiplier = .01f);
bool SliderDoubleW(
    const char* label, double* v, double v_min, double v_max,
    std::function<void(double*)> cb = nullptr,
    const char* format = "%.3f",
    ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp,
    double multiplier = .001f);

}

#endif // !TB_INTERNAL_H
