#include <SDL.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>
#include "color.h"
#include "framebuffer.h"
#include "obj_loader.h"
#include "triangle.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

std::vector<Vec3> vertices;
std::vector<Face> faces;

// Transformacion modelo -> pantalla (vista superior, plano X-Z).
float centerX, centerZ, scale;

// Calcula el centro de la caja envolvente y una escala uniforme para que
// el modelo ocupe el 80% del lado mas restrictivo de la pantalla.
void computeTransform() {
    float minX = vertices[0].x, maxX = vertices[0].x;
    float minZ = vertices[0].z, maxZ = vertices[0].z;
    for (const Vec3& v : vertices) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minZ = std::min(minZ, v.z); maxZ = std::max(maxZ, v.z);
    }
    centerX = (minX + maxX) / 2.0f;
    centerZ = (minZ + maxZ) / 2.0f;
    scale = 0.8f * std::min(SCREEN_WIDTH / (maxX - minX), SCREEN_HEIGHT / (maxZ - minZ));
}

// Proyeccion ortografica vista desde arriba: se descarta Y (altura).
// Blender exporta con Y hacia arriba y -Z hacia adelante, asi que en la
// vista superior Z crece hacia abajo de la pantalla, igual que y_pantalla.
Point toScreen(const Vec3& v) {
    int x = (int)(SCREEN_WIDTH / 2.0f + (v.x - centerX) * scale);
    int y = (int)(SCREEN_HEIGHT / 2.0f + (v.z - centerZ) * scale);
    return Point{x, y};
}

// Recorre todas las caras y dibuja sus triangulos. Devuelve cuantos dibujo.
int render(Framebuffer& fb) {
    int drawn = 0;
    for (const Face& face : faces) {
        const std::vector<int>& idx = face.vertexIndices;

        // Una cara de n vertices se divide en abanico en n-2 triangulos:
        // (0,1,2), (0,2,3), ... Para un triangulo es una sola iteracion.
        for (size_t k = 1; k + 1 < idx.size(); ++k) {
            int i0 = idx[0];
            int i1 = idx[k];
            int i2 = idx[k + 1];

            if (i0 < 0 || i1 < 0 || i2 < 0 ||
                i0 >= (int)vertices.size() || i1 >= (int)vertices.size() || i2 >= (int)vertices.size()) {
                continue;
            }

            const Vec3& v0 = vertices[i0];
            const Vec3& v1 = vertices[i1];
            const Vec3& v2 = vertices[i2];

            triangle(fb, toScreen(v0), toScreen(v1), toScreen(v2));
            ++drawn;
        }
    }
    return drawn;
}

int main(int argc, char* argv[]) {
    // Uso: render_obj [modelo.obj] [--screenshot]
    // Con --screenshot guarda el primer frame en render.bmp y termina.
    const char* modelPath = "NaveEspacial.obj";
    bool screenshotAndExit = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--screenshot") == 0) screenshotAndExit = true;
        else modelPath = argv[i];
    }
    if (!loadOBJ(modelPath, vertices, faces) || vertices.empty()) {
        std::fprintf(stderr, "No se pudo cargar el modelo: %s\n", modelPath);
        return 1;
    }
    computeTransform();
    std::printf("Modelo: %zu vertices, %zu caras\n", vertices.size(), faces.size());

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Proyecto 1 - Modelo OBJ",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window || !renderer || !texture) {
        std::fprintf(stderr, "Error creando ventana/renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    Framebuffer fb(SCREEN_WIDTH, SCREEN_HEIGHT);
    fb.setClearColor(Color{0, 0, 0});

    bool running = true;
    bool firstFrame = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
                if (fb.saveBMP("render.bmp")) std::printf("Captura guardada en render.bmp\n");
            }
        }

        fb.clear();
        fb.setColor(Color{255, 255, 0});
        int drawn = render(fb);
        fb.renderBuffer(renderer, texture);

        if (firstFrame) {
            std::printf("Triangulos dibujados: %d\n", drawn);
            std::fflush(stdout);
            firstFrame = false;
            if (screenshotAndExit) {
                fb.saveBMP("render.bmp");
                running = false;
            }
        }

        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
