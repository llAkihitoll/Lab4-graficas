#include <SDL.h>
#include <algorithm>
#include <cmath>
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

// Angulos de la camara en grados: giro alrededor del eje vertical (Y) y
// inclinacion hacia abajo (eje X). Juntos dan una vista en diagonal.
const float YAW_DEG = -40.0f;
const float PITCH_DEG = 30.0f;

// Vertices ya rotados: x hacia la derecha, y hacia arriba, z hacia la camara.
std::vector<Vec3> rotated;

// Transformacion modelo -> pantalla.
float centerX, centerY, scale;

Vec3 rotate(const Vec3& v) {
    const float yaw = YAW_DEG * 3.14159265f / 180.0f;
    const float pitch = PITCH_DEG * 3.14159265f / 180.0f;

    // Giro alrededor de Y.
    float x1 = v.x * std::cos(yaw) + v.z * std::sin(yaw);
    float z1 = -v.x * std::sin(yaw) + v.z * std::cos(yaw);

    // Inclinacion alrededor de X: la camara mira el modelo desde arriba.
    float y2 = v.y * std::cos(pitch) - z1 * std::sin(pitch);
    float z2 = v.y * std::sin(pitch) + z1 * std::cos(pitch);
    return Vec3{x1, y2, z2};
}

// Rota todos los vertices y calcula el centro de la caja envolvente y una
// escala uniforme para que el modelo ocupe el 80% del lado mas restrictivo.
void computeTransform() {
    rotated.clear();
    for (const Vec3& v : vertices) rotated.push_back(rotate(v));

    float minX = rotated[0].x, maxX = rotated[0].x;
    float minY = rotated[0].y, maxY = rotated[0].y;
    for (const Vec3& v : rotated) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
    }
    centerX = (minX + maxX) / 2.0f;
    centerY = (minY + maxY) / 2.0f;
    scale = 0.8f * std::min(SCREEN_WIDTH / (maxX - minX), SCREEN_HEIGHT / (maxY - minY));
}

// Proyeccion ortografica de un vertice rotado: se descarta z. En pantalla
// la y crece hacia abajo, por eso se invierte.
Point toScreen(const Vec3& v) {
    int x = (int)(SCREEN_WIDTH / 2.0f + (v.x - centerX) * scale);
    int y = (int)(SCREEN_HEIGHT / 2.0f - (v.y - centerY) * scale);
    return Point{x, y};
}

struct Tri {
    int i0, i1, i2;
    float depth;      // z promedio; mayor z = mas cerca de la camara
    float intensity;  // brillo segun la orientacion de la cara
};

// Recorre todas las caras y dibuja sus triangulos. Devuelve cuantos dibujo.
int render(Framebuffer& fb, Color baseColor) {
    // Luz que viene desde arriba, la izquierda y la camara.
    Vec3 light{-0.4f, 0.7f, 0.6f};
    float len = std::sqrt(light.x * light.x + light.y * light.y + light.z * light.z);
    light = Vec3{light.x / len, light.y / len, light.z / len};

    std::vector<Tri> tris;
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

            const Vec3& v0 = rotated[i0];
            const Vec3& v1 = rotated[i1];
            const Vec3& v2 = rotated[i2];

            // Normal de la cara = (v1 - v0) x (v2 - v0).
            Vec3 a{v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
            Vec3 b{v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
            Vec3 n{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
            float nlen = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
            float d = nlen > 0 ? std::fabs(n.x * light.x + n.y * light.y + n.z * light.z) / nlen : 0;

            tris.push_back(Tri{i0, i1, i2, (v0.z + v1.z + v2.z) / 3.0f, 0.25f + 0.75f * d});
        }
    }

    // Algoritmo del pintor: primero los triangulos mas lejanos, para que
    // los cercanos queden encima.
    std::sort(tris.begin(), tris.end(), [](const Tri& p, const Tri& q) { return p.depth < q.depth; });

    for (const Tri& t : tris) {
        fb.setColor(Color{(uint8_t)(baseColor.r * t.intensity),
                          (uint8_t)(baseColor.g * t.intensity),
                          (uint8_t)(baseColor.b * t.intensity)});
        triangle(fb, toScreen(rotated[t.i0]), toScreen(rotated[t.i1]), toScreen(rotated[t.i2]));
    }
    return (int)tris.size();
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
        int drawn = render(fb, Color{255, 255, 0});
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
