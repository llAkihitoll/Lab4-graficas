#include "triangle.h"
#include <algorithm>

// Producto cruz 2D de (b - a) x (p - a): indica de que lado de la
// arista AB queda el punto P.
static int edge(Point a, Point b, int px, int py) {
    return (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x);
}

void triangle(Framebuffer& fb, Point a, Point b, Point c) {
    // Caja envolvente del triangulo, recortada a la pantalla.
    int minX = std::max(0, std::min({a.x, b.x, c.x}));
    int maxX = std::min(fb.width() - 1, std::max({a.x, b.x, c.x}));
    int minY = std::max(0, std::min({a.y, b.y, c.y}));
    int maxY = std::min(fb.height() - 1, std::max({a.y, b.y, c.y}));

    for (int py = minY; py <= maxY; ++py) {
        for (int px = minX; px <= maxX; ++px) {
            int w0 = edge(b, c, px, py);
            int w1 = edge(c, a, px, py);
            int w2 = edge(a, b, px, py);
            // Dentro si el punto esta del mismo lado de las tres aristas;
            // se aceptan ambos sentidos de giro (horario y antihorario).
            bool inside = (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
            if (inside) fb.point(px, py);
        }
    }
}
