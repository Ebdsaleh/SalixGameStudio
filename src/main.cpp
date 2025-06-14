#include <SDL.h>
#include <iostream>


int main(int argc, char* argv[]) { 
    
    // 1. Initialize SDL
    if ((SDL_INIT_VIDEO) < 0 ) {
        std::cout << "SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. Create a Window 1280 x 720
    SDL_Window* window = SDL_CreateWindow(
        "Salix Game Studio",        // Window title
        SDL_WINDOWPOS_CENTERED,     // Initial x position
        SDL_WINDOWPOS_CENTERED,     // Initial y position
        1280,                       // Width in pixels
        720,                        // Height in pixels
        SDL_WINDOW_SHOWN            // Flags
    );

    if (window == nullptr) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 3. Game Loop
    bool is_running = true;
    while (is_running) {
        SDL_Event event;
        // Check for events
        while (SDL_PollEvent(&event)) {
            // Check if the user wants to quit
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }
    }

    // 4. Shutdown and Clean up
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}