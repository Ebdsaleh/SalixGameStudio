// Tests/TestFixtures.h
#pragma once
#include <SDL.h>
#include <iostream>

// Add Windows headers to query and set the timer resolution directly
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

// --- For the ImGui fixture ---
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>


struct HighResolutionTimerFixture {
    HighResolutionTimerFixture() {
        std::cout << "\n--- [FIXTURE SETUP] ---\n";
        
        // Explicitly request 1ms timer resolution. This is the key change.
        if (timeBeginPeriod(1) == TIMERR_NOERROR) {
            std::cout << "[FIXTURE INFO] High-resolution timer (1ms) requested successfully.\n";
        } else {
            std::cerr << "[FIXTURE WARNING] Could not set high-resolution timer.\n";
        }

        if (SDL_Init(SDL_INIT_TIMER) != 0) {
            std::cerr << "[FIXTURE ERROR] SDL_Init(SDL_INIT_TIMER) failed: " << SDL_GetError() << std::endl;
        }
    }

    ~HighResolutionTimerFixture() {
        //SDL_Quit();
        
        // Explicitly release the high-resolution timer.
        timeEndPeriod(1);
        
        std::cout << "--- [FIXTURE TEARDOWN] ---\n";
    }
};


struct ImGuiTestFixture {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    ImGuiTestFixture() {
        // 1. Initialize SDL
        SDL_Init(SDL_INIT_VIDEO);

        // --- THIS IS THE FIX ---
        // Added SDL_WINDOW_OPENGL flag, which is required by the ImGui backends.
        window = SDL_CreateWindow("ImGui Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
        // ---------------------

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // 2. Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        // 3. Build font atlas
        io.Fonts->AddFontDefault();
        io.Fonts->Build();
        ImGui_ImplSDLRenderer2_UpdateTexture(io.Fonts->TexData);
    }
    ~ImGuiTestFixture() {
        // Shutdown in reverse order
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // Helper to start a new frame for testing
    void new_test_frame() {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }
};