// =================================================================================
// Filename:    Salix/rendering/OpenGLRenderer.h
// Author:      SalixGameStudio
// Description: Implements the IRenderer interface using modern OpenGL.
// =================================================================================
#pragma once
#include <Salix/rendering/IRenderer.h>
#include <memory>

// Forward declare SDL types to avoid including SDL headers here
struct SDL_Window;
typedef void* SDL_GLContext;

namespace Salix {

    class OpenGLRenderer : public IRenderer
    {
    public:
        OpenGLRenderer();
        virtual ~OpenGLRenderer() override;

        // --- IRenderer Interface Implementation ---
        bool initialize(const WindowConfig& config) override;
        void shutdown() override;
        void begin_frame() override;
        void end_frame() override;
        void clear() override;
        IWindow* get_window() override;
        void* get_native_handle() override;
        // --- End Interface Implementation ---

    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
    };

} // namespace Salix
