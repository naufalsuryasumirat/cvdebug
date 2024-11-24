#include "trackbar.h"

using fmt::color;
using fmt::fg;
using fmt::bg;

namespace {

bool g_initially_debugging;
std::weak_ptr<bool> g_debugging;

}

void tb::init(std::shared_ptr<bool> debugging)
{
    ::g_debugging = debugging;
}

void tb::deinit()
{
}

void tb::update()
{
}

void tb::view()
{
}
