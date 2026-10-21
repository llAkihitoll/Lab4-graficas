#pragma once
#include <string>
#include <vector>

struct Vec3 {
    float x, y, z;
};

// Una cara guarda los indices (base 0) de sus vertices. Puede tener
// 3 o mas vertices; el modelo de la nave usa cuadrilateros.
struct Face {
    std::vector<int> vertexIndices;
};

// Lee las lineas "v" y "f" del archivo. Ignora texturas, normales y
// materiales porque este ejercicio solo necesita la geometria.
bool loadOBJ(const std::string& path, std::vector<Vec3>& vertices, std::vector<Face>& faces);
