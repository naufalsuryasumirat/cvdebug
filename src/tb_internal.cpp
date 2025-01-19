#include <algorithm>
#include <deque>
#include <type_traits>

#include "imgui.h"
#include "imgui/imgui_internal.h"
#include "tb_internal.h"

namespace {

void _dbg_p(const tb::p& param, uint16_t depth) {
    fmt::println("d[{:02}]: {}, child: {}", depth, param.name, param.children.size());
    if (param.has_data) {
        std::visit([](auto&& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int*>) {
                fmt::println("\t-> int: {}", *v);
            } else if constexpr (std::is_same_v<T, float*>) {
                fmt::println("\t-> float: {:.2f}", *v);
            } else if constexpr (std::is_same_v<T, double*>) {
                fmt::println("\t-> double: {:.3f}", *v);
            } else if constexpr (std::is_same_v<T, bool*>) {
                fmt::println("\t-> bool: {}", *v);
            } else { fmt::println("\t-> unknown"); }
        }, param.d.v);
    }
}

constexpr float default_multiplier_f = 0.1f;

}

void tbd::bfsp(const tb::p& params)
{
    std::deque<std::pair<uint16_t, const tb::p*>> d = {{0,&params}};
    while (d.size()) {
        auto& [depth, param] = d.front();
        ::_dbg_p(*param, depth);
        for (const auto& c : param->children)
            d.push_back({depth+1,&c});
        d.pop_front();
    }
}

void tbd::dfsp(const tb::p& params, const uint16_t depth)
{
    ::_dbg_p(params, depth);
    for (size_t i = 0; i < params.children.size(); ++i) {
        dfsp(params.children[i], depth+1);
    }
}

bool ImGui::SliderIntW(
    const char* label, int* v, int v_min, int v_max,
    std::function<void(int*)> cb,
    const char* format, ImGuiSliderFlags flags, int incrementer)
{
    bool changed = ImGui::SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
    int temp = *v;

    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
    if (ImGui::IsItemHovered()) {
        // NOTE: could be way simpler
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel) {
            if (ImGui::IsItemActive()) {
                ImGui::ClearActiveID();
            } else {
                temp = *v + (wheel > 0.0 ? incrementer : -incrementer);
                changed = true;
            }
        }
    }

    if (changed) {
        if (cb != nullptr)
            cb(&temp);
        if ((flags | ImGuiSliderFlags_AlwaysClamp) == flags)
            temp = std::clamp(temp, v_min, v_max);
        changed = temp != *v;
        *v = temp;
    }

    return changed;
}

bool ImGui::SliderFloatW(
    const char* label, float* v, float v_min, float v_max,
    std::function<void(float*)> cb,
    const char* format, ImGuiSliderFlags flags, float multiplier)
{
    bool changed = ImGui::SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
    float temp = *v;

    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
    if (ImGui::IsItemHovered()) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel) {
            if (ImGui::IsItemActive()) {
                ImGui::ClearActiveID();
            } else {
                const float _multiplier = ImGui::IsKeyDown(ImGuiKey_ModShift) ? multiplier : ::default_multiplier_f;
                temp = *v + (wheel * _multiplier);
                changed = true;
            }
        }
    }

    if (changed) {
        if (cb != nullptr)
            cb(&temp);
        if ((flags | ImGuiSliderFlags_AlwaysClamp) == flags)
            temp = std::clamp(temp, v_min, v_max);
        changed = temp != *v;
        *v = temp;
    }

    return changed;
}

bool ImGui::SliderDoubleW(
    const char* label, double* v, double v_min, double v_max,
    std::function<void(double*)> cb,
    const char* format, ImGuiSliderFlags flags, double multiplier)
{
    bool changed = ImGui::SliderScalar(label, ImGuiDataType_Double, v, &v_min, &v_max, format, flags);
    double temp = *v;

    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
    if (ImGui::IsItemHovered()) {
        const double wheel = ImGui::GetIO().MouseWheel;
        if (wheel) {
            if (ImGui::IsItemActive()) {
                ImGui::ClearActiveID();
            } else {
                const double _multiplier = ImGui::IsKeyDown(ImGuiKey_ModShift) ? multiplier : ::default_multiplier_f;
                temp = *v + (wheel * _multiplier);
                changed = true;
            }
        }
    }

    if (changed) {
        if (cb != nullptr)
            cb(&temp);
        if ((flags | ImGuiSliderFlags_AlwaysClamp) == flags)
            temp = std::clamp(temp, v_min, v_max);
        changed = temp != *v;
        *v = temp;
    }

    return changed;
}
