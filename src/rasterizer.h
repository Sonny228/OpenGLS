//растеризация треугольника на барицентрических координатах + z-buffer.
#pragma once
#include <tgaimage.h>
#include <vector>
#include "geometry.h"
#include "shader.h"

// Барицентрические координаты точки (TochkaX, TochkaY) относительно треугольника
Vec3 barycentric(const Vec3 Tochki[3], float TochkaX, float TochkaY);                                                                         // Барицентрические координаты
// Нарисовать один треугольник и запустить фрагментный шейдер
void triangle(const Vec4 VershinyClip[3], IShader& Shejder, TGAImage& Kartinka,                                                                            // Растеризатор
              std::vector<float>& BufferGlubiny);
