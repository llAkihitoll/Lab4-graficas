#pragma once
#include "framebuffer.h"

// Punto en coordenadas de pantalla (pixeles).
struct Point {
    int x, y;
};

// Rellena el triangulo ABC con el color actual del framebuffer.
void triangle(Framebuffer& fb, Point a, Point b, Point c);
