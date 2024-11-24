#include <memory>
// fmt includes
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <fmt/args.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <SDL.h>
#include <SDL_opengl.h>

// should adhere to ELM architecture as much as possible
namespace tb {

std::shared_ptr<bool> init(const bool init_state);

void update();

void view();

void deinit();

}
