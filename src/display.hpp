#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <cstdint>

#include "core.hpp"

class Display {
public:
    bool initialize();
    void render(const std::array<byte, DISPLAY_WIDTH * DISPLAY_HEIGHT>& framebuffer);
    void shutdown();

private:
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
    SDL_Texture* texture_{};

    std::array<std::uint32_t, DISPLAY_WIDTH * DISPLAY_HEIGHT> pixels_{};
};
