#include "framebuffer.h"
#include <algorithm>

Framebuffer::Framebuffer(int width, int height)
    : w(width), h(height), pixels((size_t)width * (size_t)height),
      clearColor(0xFF000000u), currentColor(0xFFFFFFFFu) {
    clear();
}

void Framebuffer::setClearColor(Color color) {
    clearColor = color.toARGB();
}

void Framebuffer::setColor(Color color) {
    currentColor = color.toARGB();
}

void Framebuffer::clear() {
    std::fill(pixels.begin(), pixels.end(), clearColor);
}

void Framebuffer::point(int x, int y) {
    if (x < 0 || y < 0 || x >= w || y >= h) return;
    pixels[(size_t)y * w + x] = currentColor;
}

void Framebuffer::renderBuffer(SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_UpdateTexture(texture, nullptr, pixels.data(), w * (int)sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

// Guarda el contenido actual del framebuffer como imagen BMP.
bool Framebuffer::saveBMP(const char* path) const {
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        (void*)pixels.data(), w, h, 32, w * (int)sizeof(uint32_t), SDL_PIXELFORMAT_ARGB8888);
    if (!surface) return false;
    bool ok = SDL_SaveBMP(surface, path) == 0;
    SDL_FreeSurface(surface);
    return ok;
}
