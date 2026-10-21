#pragma once
#include <SDL.h>
#include <cstdint>
#include <vector>
#include "color.h"

// Arreglo de pixeles ARGB8888 donde se dibuja todo. SDL solo se usa
// en renderBuffer() para copiar el arreglo a una textura y mostrarlo.
class Framebuffer {
public:
    Framebuffer(int width, int height);

    void setClearColor(Color color);
    void setColor(Color color);
    void clear();
    void point(int x, int y);

    void renderBuffer(SDL_Renderer* renderer, SDL_Texture* texture);
    bool saveBMP(const char* path) const;

    int width() const { return w; }
    int height() const { return h; }

private:
    int w, h;
    std::vector<uint32_t> pixels;
    uint32_t clearColor;
    uint32_t currentColor;
};
