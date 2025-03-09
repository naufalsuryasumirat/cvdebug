#include <SDL.h>
#include <SDL_opengl.h>
#include <opencv.hpp>
#include <opencv2/core.hpp>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>

#include "SDL_video.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "implot/implot.h"
#include "opencv2/core/base.hpp"

#include "tb_internal.h"
#include "trackbar.h"

#include "util/util.h"

using fmt::color;
using fmt::fg;
using fmt::bg;

namespace tb {

void deinit();
void slider_from(const tb::p& _p);
void init_textures();
void render_parameters(const tb::p& root, uint16_t depth = 0);
void render_trackbar();
void render_canvas();
void render_metrics();
void render_cursor_metrics();
void render_histogram(const cv::Mat& src, const cv::Mat& mask = cv::Mat());
void key_events();
const size_t cur_i();

}

namespace {
bool initially_debugging;
std::weak_ptr<bool> debugging;
std::unique_ptr<tb::p> params; // the unique_ptr is actually useless if global

SDL_Window* window;
SDL_GLContext gl_context;
// TODO: figure out how to imbue uniqueness into the semantics itself, don't rely on people treating as unique
ImGuiIO* io;
// NOTE: this might not be a good implementation, but might be good for efficiency

// should handle all types of cv::Mat
// NOTE: hate to say it, but images might need to be a wrapper class or struct
// NOTE: cur_images and max_images might not be needed
std::vector<cv::Mat> images;
int show_i = 0,
    show_last = show_i;

// OpenGL textures
GLuint tex_canvas;

// states
bool always_center = true;

void cleanup(bool* b)
{
    tb::deinit();
}

void stop_debugging()
{
    if (::debugging.expired())
        return;
    std::shared_ptr<bool> p_debugging = ::debugging.lock();
    *p_debugging = false;
}

void reset()
{
    ::show_last = static_cast<int>(::images.size());
    ::images.clear(); // NOTE: maybe should reserve?
}

}

// TODO: init should probably have an optional cleanup function involved
std::shared_ptr<bool> tb::init(const bool init_state, const tb::p& _p)
{
    if (!init_state) {
        std::shared_ptr<bool> p_debugging = std::make_shared<bool>(init_state);
        ::initially_debugging = false;
        ::debugging = p_debugging;
        return p_debugging;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        fmt::print(fg(color::red), "Error: {}\n", SDL_GetError());
        return nullptr;
    }

    const char* glsl_version = "#version 330"; // default: 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    ::window = SDL_CreateWindow( "debug", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
    if (::window == nullptr) {
        fmt::print(fg(color::red), "Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return nullptr;
    }

    ::gl_context = SDL_GL_CreateContext(::window);
    SDL_GL_MakeCurrent(::window, ::gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    // FIX: this is passing by value, ideally should be a unique pointer to the only availalbe IO
    // ::io = std::make_unique<ImGuiIO>(ImGui::GetIO()); (void)(::io.get());
    ::io = &ImGui::GetIO(); (void)(*::io);
    ::io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ::io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ::io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ::io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    //::io->ConfigViewportsNoAutoMerge = true;
    //::io->ConfigViewportsNoTaskBarIcon = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (::io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        constexpr ImVec2 min_size = { 350, 350 };
        style.WindowMinSize = min_size;
    }

    ImGui_ImplSDL2_InitForOpenGL(::window, ::gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // TODO: should make this a user function
    // ::io->IniFilename = nullptr;

    // trackbar state
    ::initially_debugging = init_state;
    std::shared_ptr<bool> p_debugging(new bool(init_state), ::cleanup);
    ::debugging = p_debugging;
    ::params = std::make_unique<p>(_p);

    tb::init_textures();

    return p_debugging;
}

// TODO: maybe also check if shared_ptr is expired
void tb::update()
{
    if (!::debugging.lock()) {
        fmt::print(fg(color::white_smoke) | bg(color::pink), "update called before a call to init\n");
        return;
    }
    if (!*::debugging.lock())
        return;

    // TODO: how to simulate continue without a loop?
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            ::stop_debugging();
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            ::stop_debugging();
    }
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
        // NOTE: can't test this w/ i3-wm
        // TODO: debounce by saving is_minimized (or similar) state globally
        SDL_Delay(10); // debouncing?
        // continue;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    tb::render_trackbar();
}

void tb::view()
{
    if (!::debugging.lock()) {
        fmt::print(fg(color::white_smoke) | bg(color::pink), "view called before a call to init\n");
        return;
    }
    if (!*::debugging.lock())
        return;

    tb::render_canvas();
    tb::render_metrics();
    // TODO: render fps
    // TODO: render frozen screenshot

    ImGui::Render();

    glViewport(0, 0, (int)::io->DisplaySize.x, (int)::io->DisplaySize.y);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (::io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(::window);

    ::reset();
}

void tb::deinit()
{
    if (!::initially_debugging)
        return;

    ::initially_debugging = false;

    // OpenGL cleanup
    glDeleteTextures(1, &::tex_canvas);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(::gl_context);
    SDL_DestroyWindow(::window);
    SDL_Quit();

    ::io = nullptr;
}

void tb::show(const cv::Mat& _image)
{
    // TODO: convert to tb image struct
    // NOTE: probably should also process the image here
    ::images.emplace_back(_image.clone());
}

// NOTE: ideally, this should also stack?
void tb::show(const std::vector<cv::Mat>& _images, bool is_horizontal)
{
    // TODO: Mat types, cast it to the biggest type
    // NOTE: should probably ignore whether they're they same size or not
    if (_images.empty())
        ; // TODO: do something that makes sense when it's empty
    if (_images.size() == 1)
        tb::show(_images[0]);

    // NOTE: std::for_each might be a little more efficient?
    int inc = 0;
    if (is_horizontal) {
        const int& max_row = std::max_element(
            _images.begin(), _images.end(),
            [](const cv::Mat& _1, const cv::Mat& _2) -> int {
                return _1.rows < _2.rows;
            })->cols;
        inc = max_row;
    } else {
        const int& max_col = std::max_element(
            _images.begin(), _images.end(),
            [](const cv::Mat& _1, const cv::Mat& _2) -> int {
                return _1.cols < _2.cols;
            })->rows;
        inc = max_col;
    }

    // TODO: delete this
    fmt::println("inc: {0}", inc);
}

// TODO: implement grid concat
void tb::show(const std::vector<std::vector<cv::Mat>>& _images)
{
}

void tb::slider_from(const tb::p& _p)
{
    if (!_p.has_data)
        return;

    std::visit([&_p](auto&& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, int*>)
            ImGui::SliderIntW(_p.name.c_str(), v, _p.d.min, _p.d.max, _p.d.cb);
        else if constexpr (std::is_same_v<T, float*>)
            ImGui::SliderFloatW(_p.name.c_str(), v, _p.d.min, _p.d.max, _p.d.cb);
        else if constexpr (std::is_same_v<T, double*>)
            ImGui::SliderDoubleW(_p.name.c_str(), v, _p.d.min, _p.d.max, _p.d.cb);
        else if constexpr (std::is_same_v<T, bool*>)
            ImGui::Checkbox(_p.name.c_str(), v);
    }, _p.d.v);
}

void tb::render_parameters(const tb::p& root, uint16_t depth)
{
    bool traverse = true;
    const bool is_leaf = root.children.empty();
    if (is_leaf && !root.has_data)
        return; // TODO: write to trackbar indicate error in pp

    if (depth == 0)
        tb::slider_from({"show", {&::show_i, std::max(::show_last-1, 0)}});

    if (depth && !root.has_data)
        traverse = ImGui::CollapsingHeader(root.name.c_str());

    if (traverse && root.has_data) {
        const float indentation = 10.0f * std::max(static_cast<int>(depth)-1, 0);
        if (indentation) ImGui::Indent(indentation);
        tb::slider_from(root);
        if (indentation) ImGui::Unindent(indentation);
    }

    if (!traverse)
        return;

    std::for_each(
        root.children.begin(),
        root.children.end(),
        std::bind(tb::render_parameters, std::placeholders::_1, depth+1)
    );
}

void tb::render_trackbar()
{
    constexpr ImVec2 min_size{ 350, 800 },
                     max_size{ 900, 900 };
    ImGui::SetNextWindowSizeConstraints(min_size, max_size);
    ImGui::Begin(::params->name.c_str());

    tb::key_events();
    tb::render_parameters(*::params);

    ImGui::End();
}

void tb::render_canvas()
{
    constexpr ImVec2 min_imsize{ 820, 840 },
                     max_imsize{ 1920, 1080 };
    ImGui::SetNextWindowSizeConstraints(min_imsize, max_imsize);

    ImGui::Begin("canvas", NULL, ImGuiWindowFlags_HorizontalScrollbar);
    tb::key_events();

    // actions that will be done to the image (probably conversions)
    //  if element is of type uchar (irregardless of channel numbers), convert to RGBA
    //  generally should only care about the depth, not the channel
    //  8-bit-U: RGBA
    //  16-bit-U: force convertTo to 8-bit
    //  32-bit-U: force convertTo to 8-bit
    //  16-bit-F: upscale to 32-bit
    //  32-bit-F: inherently should support this
    //  for floating-point types: should probably only take in one channel
    //  [maybe] signed-types: for convertTo to 32-bit floating-point
    //  floating points should support color maps

    const cv::Mat& cur_image = ::images[cur_i()];
    cv::Mat tmp = util::as_rgba(cur_image);

    // OPTIMIZE: anything and everything, this is the bread and butter
    // OPTIMIZE: render only when something changes?

    // OpenGL texture identifier
    glBindTexture(GL_TEXTURE_2D, ::tex_canvas);
    // set up filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // upload pixels to texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmp.cols, tmp.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp.data);

    const ImVec2 w_size = ImGui::GetWindowSize();
    const int im_w = cur_image.cols,
              im_h = cur_image.rows;
    const bool x_available = w_size.x > im_w,
               y_available = w_size.y > im_h;

    // right-click menu for centering image, default always_center
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
        ImGui::BeginPopup("canvas_popup");
    if (ImGui::BeginPopupContextWindow("canvas_popup")) {
        ImGui::MenuItem("Center Image", "", &always_center);
        ImGui::EndPopup();
    }

    if (always_center) {
        constexpr int top_pad = 10;
        if (x_available)
            ImGui::SetCursorPosX((w_size.x - im_w) / 2);
        if (y_available)
            ImGui::SetCursorPosY(((w_size.y - im_h) / 2) + top_pad);
    }

    ImGui::Image((ImTextureID)(intptr_t)::tex_canvas, ImVec2(tmp.cols, tmp.rows));

    ImGui::End();
}

void tb::render_metrics()
{
    constexpr ImVec2 min_size{ 500, 500 },
                     max_size{ 1200, 1000 };
    ImGui::SetNextWindowSizeConstraints(min_size, max_size);
    ImGui::Begin("metrics");

    tb::key_events();
    tb::render_histogram(::images[cur_i()]);
    ImGui::Separator();
    tb::render_cursor_metrics();

    ImGui::End();
}

void tb::render_histogram(const cv::Mat& src, const cv::Mat& mask)
{
    // TODO: Combo() for LogScale(default), normalized scale([0,255], NORM_MINMAX), actual value
    constexpr ImVec2 min_size{ 300, 400 }, // might not be useful
                     max_size{ 900, 400 };
    ImGui::SetNextWindowSizeConstraints(min_size, max_size);

    ImGui::BeginChild("histogram");
    tb::key_events();

    const ImVec2 window_size = ImGui::GetWindowSize();

    std::vector<cv::Mat> hists(src.channels()),
                         norm_hists(src.channels()),
                         color_planes;
    std::vector<int> counts(src.channels());
    cv::split(src, color_planes);
    constexpr int hist_size = 256;
    float range[] = { 0, 256 };
    const float* hist_range[] = { range };
    std::for_each(color_planes.begin(), color_planes.end(), [&, i=0](const cv::Mat& plane) mutable {
        cv::calcHist(&plane, 1, 0, mask, hists[i++], 1, &hist_size, hist_range);
    });

    static const std::array<float, 256> _range = []() -> std::array<float, 256> {
        std::array<float, 256> res;
        std::iota(res.begin(), res.end(), 0);
        return res;
    }();

    // NOTE: consider other scaling options
    // static int current_item = 0;
    // constexpr const char* items[3] = { "Log", "Norm", "Actual" };
    // ImGui::Combo("Scale", &current_item, items, 3);

    static bool log_scale = false;
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
        ImGui::BeginPopup("metrics_popup");
    if (ImGui::BeginPopupContextWindow("metrics_popup")) {
        ImGui::MenuItem("Log Scale", "", &log_scale);
        ImGui::EndPopup();
    }

    static constexpr std::array<ImVec4, 4> line_colors = { ImVec4{0,0,1,1}, ImVec4{0,1,0,1}, ImVec4{1,0,0,1}, ImVec4{1,1,1,1} }; // bgrw
    if (ImPlot::BeginPlot("##Histogram", ImVec2{window_size.x-30, 300}, ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_Crosshairs)) {
        if (log_scale)
            ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_SymLog);
        
        ImPlot::SetupAxesLimits(-10, 260, -10, 265);
        constexpr ImPlotAxisFlags x_flags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoHighlight;
        constexpr ImPlotAxisFlags y_flags =
            ImPlotAxisFlags_LockMin | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoHighlight;
        ImPlot::SetupAxes("color", "count", x_flags, y_flags);
        // TODO: smaller increment for x axis ticks

        const ImPlotPoint axis_pos = ImPlot::GetPlotMousePos();
        std::for_each(hists.begin(), hists.end(), [&, i=0](const cv::Mat& hist) mutable -> void {
            counts[i] = *hists[i].ptr<float>(std::clamp(static_cast<int>(axis_pos.x), 0, 255));
            cv::normalize(hists[i], norm_hists[i], 0, _range.size(), cv::NormTypes::NORM_MINMAX);
            const float* y_values = (float*)norm_hists[i].datastart;
            ImPlot::SetNextFillStyle(line_colors[i], 0.25f);
            ImPlot::PlotShaded(fmt::format("s##{}", i).c_str(), _range.data(), y_values, _range.size(), 0);
            ImPlot::SetNextLineStyle(line_colors[i]);
            ImPlot::PlotLine(fmt::format("##{}", i++).c_str(), _range.data(), y_values, _range.size());
        });

        ImPlot::EndPlot();
    }

    ImGui::Text("Histogram (value at crosshair): ");
    if (src.channels() > 1)
        ImGui::Text("Count [B,G,R]: [%d, %d, %d]", counts[0], counts[1], counts[2]);
    else
        ImGui::Text("Count: %d", counts[0]);

    ImGui::EndChild();
}

void tb::render_cursor_metrics()
{
    // TODO: cursor metrics (B,G,R) value, probably the cursor position should be collected by render_canvas
    ImGui::BeginChild("cursor_metrics");
    tb::key_events();

    const ImVec2 cursor_pos = ImGui::GetCursorPos(),
                 mouse_pos = ImGui::GetMousePos();
    ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByWindow(ImGui::FindWindowByName("canvas"));
    const ImVec2ih canvas_pos = settings->Pos;
    ImGuiID hovered_id = ImGui::GetHoveredID();
    ImGui::Text("hovered_id: %d", (int)(unsigned int)hovered_id);
    ImGui::Text("cursor_pos[x,y]: %d, %d", static_cast<int>(cursor_pos.x), static_cast<int>(cursor_pos.y));
    ImGui::Text("canvas_pos[x,y]: %d, %d", static_cast<int>(canvas_pos.x), static_cast<int>(canvas_pos.y));
    ImGui::Text("mouse_pos[x,y]: %d, %d", static_cast<int>(mouse_pos.x), static_cast<int>(mouse_pos.y));

    ImGui::EndChild();
}

void tb::init_textures()
{
    glGenTextures(1, &::tex_canvas);
}

void tb::key_events()
{
    // TODO: other key events, modifying show_i, and other user-defined key events
    // TODO: user-defined key-controls, provide common behaviors (sig_exit, sig_halt?)
    if (ImGui::IsWindowFocused()) {
        if (ImGui::IsKeyDown(ImGuiKey_ModShift) && ImGui::IsKeyPressed(ImGuiKey_Q)) {
            std::shared_ptr<bool> p_debugging = ::debugging.lock();
            *p_debugging = false;
        }
    }
}

// FIXME: very bad design, ideally this should be a callback
const size_t tb::cur_i()
{
    ::show_i = std::min(::show_i, static_cast<int>(::images.size())-1);
    return static_cast<size_t>(::show_i);
}
