#include "obj_loader.h"
#include <fstream>
#include <sstream>

bool loadOBJ(const std::string& path, std::vector<Vec3>& vertices, std::vector<Face>& faces) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            Vec3 v;
            iss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (type == "f") {
            Face face;
            std::string token;
            // Cada token tiene la forma v, v/vt, v//vn o v/vt/vn;
            // solo interesa el primer numero (indice del vertice).
            while (iss >> token) {
                int index = std::stoi(token.substr(0, token.find('/')));
                // El OBJ empieza en 1; los indices negativos son relativos
                // al final de la lista de vertices leida hasta ahora.
                if (index > 0) index -= 1;
                else index = (int)vertices.size() + index;
                face.vertexIndices.push_back(index);
            }
            faces.push_back(face);
        }
    }
    return true;
}
