#include "trackbar.h"

using fmt::color;
using fmt::fg;
using fmt::bg;

namespace {

bool initially_debugging;
std::weak_ptr<bool> debugging;

SDL_Window* window;
SDL_GLContext gl_context;

void cleanup(bool* b)
{
    tb::deinit();
}

}

std::shared_ptr<bool> tb::init(const bool init_state)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        fmt::print(fg(color::red), "Error: {}\n", SDL_GetError());
        return nullptr;
    }

    const char* glsl_version = "#version 130";
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
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    // SDL_Window* window = SDL_CreateWindow(
    ::window = SDL_CreateWindow(
        "Dear ImGui SDL2+OpenGL3 example",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, window_flags);
    // TODO: return value?
    if (::window == nullptr) {
        fmt::print(fg(color::red), "Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return nullptr;
    }

    ::gl_context = SDL_GL_CreateContext(::window);
    SDL_GL_MakeCurrent(::window, ::gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplSDL2_InitForOpenGL(::window, ::gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.IniFilename = nullptr; // disables loading ini settings for imgui

    // trackbar state
    ::initially_debugging = init_state;
    std::shared_ptr<bool> p_debugging(new bool(init_state), ::cleanup);
    ::debugging = p_debugging;

    return p_debugging;
}

// TODO: maybe also check expired
void tb::update()
{
    if (!::debugging.lock()) {
        fmt::print(fg(color::red) | bg(color::cyan), "update called before a call to init\n");
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void tb::view()
{
    if (!::debugging.lock()) {
        fmt::print(fg(color::red) | bg(color::cyan), "view called before a call to init\n");
        return;
    }

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); // (void)io;

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(window);
}

void tb::deinit()
{
    if (!::initially_debugging)
        return;

    ::initially_debugging = false;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(::gl_context);
    SDL_DestroyWindow(::window);
    SDL_Quit();
}
