#include "display.hpp"


bool Display::initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init(...) failed: " << SDL_GetError() << '\n';

        return false;
    }

    if (!SDL_CreateWindowAndRenderer("chip-8-emu", DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE, 0, &window_, &renderer_)) {
        std::cerr << "SDL_CreateWindowAndRenderer(...) failed: " << SDL_GetError() << '\n';
        SDL_Quit();

        return false;
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    if (!texture_) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << '\n';
        shutdown();

        return false;
    }

    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);

    return true;
}

void Display::render(const std::array<byte, DISPLAY_WIDTH * DISPLAY_HEIGHT>& framebuffer)
{
    for (size_t i{}; i < framebuffer.size(); ++i) {
        // White pixel if CHIP-8 pixel is on.
        // Black pixel if it is off.
        pixels_[i] = framebuffer[i] ? 0xFFFFFFFF : 0x000000FF;
    }

    SDL_UpdateTexture(
        texture_,
        nullptr,
        pixels_.data(),
        DISPLAY_WIDTH * sizeof(std::uint32_t)
    );

    SDL_RenderClear(renderer_);

    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);

    SDL_RenderPresent(renderer_);
}

void Display::shutdown()
{
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}
