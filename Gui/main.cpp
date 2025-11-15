#define IMGUI_DEFINE_MATH_OPERATORS
//#define IMGUI_IMPL_OPENGL_LOADER_GL3W

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <SDL.h>
#include <iostream>
#include <vector>
#include <GL/gl3w.h>
#include <windows.h>
#include <fstream>
#include "MouseTracker.h"

void CreateDefaultImGuiIni()
{
    std::ofstream iniFile("imgui.ini");

    if (iniFile.is_open())
    {
        iniFile << "[Window][Console]\n";
        iniFile << "Pos=5,335\n";
        iniFile << "Size=560,450\n";
        iniFile << "Collapsed=0\n\n";
        
        iniFile << "[Window][Trajectory]\n";
        iniFile << "Pos=575,25\n";
        iniFile << "Size=620,760\n";
        iniFile << "Collapsed=0\n\n";
        
        iniFile << "[Window][Mouse Tracker]\n";
        iniFile << "Pos=5,25\n";
        iniFile << "Size=560,300\n";
        iniFile << "Collapsed=0\n\n";

        iniFile << "[Table][0x0C727C7C,3]\n";
        iniFile << "RefScale=13\n";
        iniFile << "Column 0  Width=65\n";
        iniFile << "Column 1  Width=65\n";
        iniFile << "Column 2  Width=65\n\n";
        
        iniFile.close();
    }
}

void ShowError(const std::string& message)
{
    std::cerr << "Error: " << message << std::endl;

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message.c_str(), nullptr);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    if (!std::filesystem::exists("imgui.ini"))
        CreateDefaultImGuiIni();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());

        return -1;
    }

#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)
    (
        SDL_WINDOW_OPENGL | 
        SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_ALLOW_HIGHDPI
    );

    SDL_Window* window = SDL_CreateWindow
    (
        "Mouse tracker",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1200, 800,
        window_flags
    );
    
    if (!window)
    {
        ShowError("Failed to create SDL window: " + std::string(SDL_GetError()));
        SDL_Quit();

        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if (!gl_context)
    {
        ShowError("Failed to create OpenGL context: " + std::string(SDL_GetError()));
        SDL_DestroyWindow(window);
        SDL_Quit();

        return -1;
    }
    
    SDL_GL_MakeCurrent(window, gl_context);

    if (gl3wInit() != 0)
    {
        ShowError("Failed to initialize OpenGL loader (gl3w)");
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return -1;
    }

    SDL_GL_SetSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    Mt::MouseTracker::GetInstance().Initialize();
    
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForOpenGL(window, gl_context))
    {
        ShowError("Failed to initialize ImGui SDL2 backend");
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return -1;
    }
    
    if (!ImGui_ImplOpenGL3_Init(glsl_version))
    {
        ShowError("Failed to initialize ImGui OpenGL3 backend");
        ImGui_ImplSDL2_Shutdown();
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return -1;
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
                done = true;
            
            if (event.type == SDL_WINDOWEVENT
                && event.window.event == SDL_WINDOWEVENT_CLOSE
                && event.window.windowID == SDL_GetWindowID(window))
            {
                done = true;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        Mt::MouseTracker::GetInstance().Show();

        ImGui::Render();
        
        int display_w, display_h;
        SDL_GL_GetDrawableSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor
        (
            clear_color.x * clear_color.w,
            clear_color.y * clear_color.w,
            clear_color.z * clear_color.w,
            clear_color.w
        );

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    Mt::MouseTracker::GetInstance().Shutdown();

    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
