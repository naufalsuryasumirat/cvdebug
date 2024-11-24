#include <memory>
// fmt includes
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

// adheres to ELM architecture as much as possible
namespace tb {

void init(std::shared_ptr<bool> debugging);

void update();

void view();

void deinit();

}
