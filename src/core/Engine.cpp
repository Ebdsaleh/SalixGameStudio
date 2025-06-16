// Engine.cpp
#include "Engine.h"
// --- NOTE ---
// All RendererTypes will be included here to allow the Engine to use a singular set of methods for rendering.
#include "../rendering/SDLRenderer.h"
// --- END NOTE ---
#include "../assets/AssetManager.h"
#include "../ecs/Sprite2D.h"
#include "../ecs/Transform.h"

// We need to include the full SDL header here to use its functions
#include <SDL.h>
#include <iostream>


Engine::Engine() {
    window = nullptr;
    renderer = nullptr;
    is_running = false;
    asset_manager = nullptr;
}

Engine::~Engine() {
    // The shutdown method will handel cleanup, but this is here for completeness.

}

bool Engine::initialize(const WindowConfig& config) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Engine::initialize - SDL could not be initialized! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }  
    
    // Create and initialize Renderer
    // --- Renderer Factory ---
    switch (config.renderer_type) {
        case RendererType::SDL:
            renderer = new SDLRenderer();         
            break;
        
        // --- Future renderer types would be handled here ---
        case RendererType::OpenGl:
            // renderer = new OpenGLRenderer(); // Example for the future
            std::cerr << "Engine::initialize - OpenGL renderer is not yet supported." << std::endl;
            // Fall through to default for cleanup
        case RendererType::Vulkan:
            // renderer = new VulkanRenderer(); // Example for the future
            std::cerr << "Engine::initialize - Vulkan renderer is not yet supported." << std::endl;
            // Fall through to default for cleanup
        case RendererType::DirectX:
            // renderer = new DirectXRenderer(); // Example for the future
            std::cerr << "Engine::initialize - DirectX renderer is not yet supported." << std::endl;
            // Fall through to default for cleanup

        default:
        // render_type is null, abort renderer creation and exit main loop.
        // Only call SDL_Quit() as the window doesn't exist yet.
        std::cerr << "Engine::initialize - Invalid or unsupported renderer type requested!" << std::endl;
        SDL_Quit();
        return false;       
    }

    // --- Delegate  initialization --
    // Ask the renderer to initialize passing the configuration it needs to create the window.
    window = renderer->initialize(config);

    if (window == nullptr) {
        std::cerr << "Engine::initialize - Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        delete renderer;
        SDL_Quit();
        return false;
    }

    // --- AssetManager Initialization ---
    // Create and initialize the AssetManager.

    asset_manager = new AssetManager();
    asset_manager->initialize(renderer);  // pass the renderer to the asset manager.

    // --- TEST: Create our parent and child Entities ---
    // Create the parent Entity (this will be our facing-direction)
    parent_entity = std::make_unique<Entity>();
    parent_entity->get_transform()->position = {200.0f, 200.0f, 0.0f};

    // Create the child Entity (this will be our visible Sprite2D)
    child_entity = std::make_unique<Entity>();
    
    // Set the child's PARENT (the parent_entity's Transform Element).
    child_entity->get_transform()->set_parent(parent_entity->get_transform());
    // Set the child's local properties relative to the parent.
    child_entity->get_transform()->scale = {0.2f, 0.2f, 0.1f};  // Make it big larger than last time.

    // Add a Sprite2D Element to the child Entity so we can see it.
    Sprite2D* test_sprite = child_entity->add_element<Sprite2D>();

    // Tell the sprite to load a texture.
    // Located in the Assets directory at the root of the project.
    std::string test_image_path = "Assets/test.png";
    test_sprite->load_texture(asset_manager, test_image_path);

    // --- Use new Properties ---
    test_sprite->color = Color::White;  // Keep the color tint normal.
    test_sprite->offset.x = -50;        // Offset the sprite 50 pixels to the left.
    test_sprite->flip_h = false;         // Flip sprite horizontally.
    test_sprite->pivot = {0.0f, 0.0f };
    // --- END TEST ---

    is_running = true;
    std::cout << "Engine initialized successfully." << std::endl;
    return true;
}

void Engine::run() {
    while (is_running) {
        process_input();
        update();
        render();
    }
}

void Engine::shutdown() {
    std::cout << "Shutting down engine." << std::endl;

    // Shutdown subsystems in reverse order of creatation (LIFO).
    if (asset_manager){
        asset_manager->shutdown();
        delete asset_manager;
    }
    
    if (renderer) {
        renderer->shutdown();
        delete renderer;
    }

    if (window) {
        SDL_DestroyWindow(window);
    }
   
    SDL_Quit();
}

void Engine::process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            is_running = false;
        }
    }
}

void Engine::update() {
    // Game logic will go here
}

void Engine::render() {
    // Safer error handling if renderer is null, just stop the engine.
    if (renderer == nullptr) {
        std::cerr << "Engine::render - Renderer is null, shuting down." << std::endl;
        is_running = false;  // Gracefully exit the main loop.
        return;
    }
    // Begin drawing the frame
    renderer->begin_frame();

    // Process rendering objects here

    // --- TEST: Only Render the child_entity ---
    if (child_entity) {
        child_entity->render(renderer);
    }

    // --- END TEST ---

    // Present the rendered frame
    renderer->end_frame();   
}