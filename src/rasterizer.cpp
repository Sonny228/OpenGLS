#include "rasterizer.h"
#include <cmath> 

                                                                                                                                            // Барицентрические координаты
Vec3 barycentric(const Vec3 Tochki[3], float TochkaX, float TochkaY) {
  // Удвоенная площадь всего треугольника(векторное произведение сторон)
  float Ploshchad = (Tochki[1].x - Tochki[0].x) * (Tochki[2].y - Tochki[0].y) -
                    (Tochki[2].x - Tochki[0].x) * (Tochki[1].y - Tochki[0].y);
  if (std::fabs(Ploshchad) < 1e-8f) { // если площадь почти 0 — треугольник вырожденный
    return Vec3(-1, 1, 1); // возвращаем координаты снаружи
  }
  // отношение площади суб-треугольника к общей площади
  float Ves0 = ((Tochki[1].x - TochkaX) * (Tochki[2].y - TochkaY) -
                (Tochki[2].x - TochkaX) * (Tochki[1].y - TochkaY)) /
               Ploshchad;
  float Ves1 = ((Tochki[2].x - TochkaX) * (Tochki[0].y - TochkaY) -
                (Tochki[0].x - TochkaX) * (Tochki[2].y - TochkaY)) /
               Ploshchad;
  float Ves2 = 1.f - Ves0 - Ves1;  // сумма весов равна 1
  return Vec3(Ves0, Ves1, Ves2);
}////

void triangle(const Vec4 VershinyClip[3], IShader& Shejder, TGAImage& Kartinka,
              std::vector<float>& BufferGlubiny) {
  int Shirina = Kartinka.width();  // шир картинки в пик
  int Vysota = Kartinka.height(); // выс картинки в пик

  //1Переводим вершины в экранные координаты (в пиксели)
  Vec3 EkranVershiny[3];
  for (int NomerVershiny = 0; NomerVershiny < 3; NomerVershiny++) {
    float W = VershinyClip[NomerVershiny].w;
    //Если W<=0,верш находится за камерой или на ее плоскости
    if (W <= 0.00001f) {                                                                                                                                    //Деление на w
      return;
    }
    //коорд от-1до1раб.перспектива
    float NormX = VershinyClip[NomerVershiny].x / W;                                                                                        // Clip -> экран (Деление на w)
    float NormY = VershinyClip[NomerVershiny].y / W;
    float NormZ = VershinyClip[NomerVershiny].z / W;
    // Переводим из диапазона [-1, 1] в пиксели [0, Shirina] и [0, Vysota]
    EkranVershiny[NomerVershiny] = Vec3((NormX + 1.f) * 0.5f * Shirina,
                                        (NormY + 1.f) * 0.5f * Vysota, NormZ);
  }

  //Инициализация границ для ограничивающего прямоугольника                                                                                        Bounding Box
  float MinX = EkranVershiny[0].x;
  float MaxX = EkranVershiny[0].x;
  float MinY = EkranVershiny[0].y;
  float MaxY = EkranVershiny[0].y;
  for (int NomerVershiny = 1; NomerVershiny < 3; NomerVershiny++) {
    if (EkranVershiny[NomerVershiny].x < MinX)
      MinX = EkranVershiny[NomerVershiny].x;
    if (EkranVershiny[NomerVershiny].x > MaxX)
      MaxX = EkranVershiny[NomerVershiny].x;
    if (EkranVershiny[NomerVershiny].y < MinY)
      MinY = EkranVershiny[NomerVershiny].y;
    if (EkranVershiny[NomerVershiny].y > MaxY)
      MaxY = EkranVershiny[NomerVershiny].y;
  }
  // Не выходим за границы картинки
  if (MinX < 0) MinX = 0;
  if (MinY < 0) MinY = 0;
  if (MaxX > Shirina - 1) MaxX = (float)(Shirina - 1);
  if (MaxY > Vysota - 1) MaxY = (float)(Vysota - 1);

  int LevayaGranica = (int)MinX;
  int PravayaGranica = (int)MaxX;
  int NizhnyayaGranica = (int)MinY;
  int VerhnyayaGranica = (int)MaxY;

                                                                                                                                                            // Растеризатор
  for (int PikselY = NizhnyayaGranica; PikselY <= VerhnyayaGranica; PikselY++) {
    for (int PikselX = LevayaGranica; PikselX <= PravayaGranica; PikselX++) {
      // Берем центр пикселя + 0.5, поэтому края лесенкой, без.сглаж
      Vec3 Baricentr =                                                                                                                                      // Барицентрики
          barycentric(EkranVershiny, PikselX + 0.5f, PikselY + 0.5f);
      if (Baricentr.x < -0.0001f || Baricentr.y < -0.0001f ||
          Baricentr.z < -0.0001f) {
        continue;
      }// чтобы между треугольниками не было дырок

       //Интерполируем глубину Z                                                                                                                                              // Интерполяция
      float Glubina = Baricentr.x * EkranVershiny[0].z +
                      Baricentr.y * EkranVershiny[1].z +
                      Baricentr.z * EkranVershiny[2].z;                                                                                                       //Интерполяция
      if (Glubina < -1.f || Glubina > 1.f) {                                                                                                          // Z-буфер (Отсечение)
        continue;
      }
      int IndeksPiksela =
          PikselY * Shirina + PikselX;  // номер пикселя в буф.глуб
      if (Glubina >= BufferGlubiny[IndeksPiksela]) {                                                                                                       // Z-буфер (Тест)
        continue;
      }

      TGAColor Tsvet;                                                                                                                           //Фрагментный шейдер (Гуро)
      if (Shejder.fragment(Baricentr, Tsvet)) {
        continue;
      }
      BufferGlubiny[IndeksPiksela] = Glubina;                                                                                                           // Z-буфер (Запись)
      Kartinka.set(PikselX, PikselY, Tsvet);
    }
  }
}
