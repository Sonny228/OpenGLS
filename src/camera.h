#pragma once
#include <algorithm>

#include "geometry.h"

class Camera {
 public:
  Vec3 Pozitsiya{0, 0, 3};
  Vec3 TochkaVzglyada{0, 0, 0};
  Vec3 VektorVverh{0, 1, 0};
  float UgolObzoraGradusy = 45.f;
  float SootnoshenieStoron = 1.f;
  float BlizhnyayaPloskost = 0.1f;              //ближе этого расстояния, камера не видит (zNear)
  float DalnyayaPloskost = 100.f;               //все,что дальше,камера не видит (zFar)

  Camera() = default;

  // Поставить камеру на сферу вокруг TochkaVzglyada: поворот влево-вправо, наклон вверх-вниз, радиус сферы.
  void orbit(float PovorotGradusy, float NaklonGradusy, float Distanciya) {
    NaklonGradusy = std::clamp(NaklonGradusy, -89.f, 89.f);  // ограничиваем наклон                                                         lookAt сломается
    float UgolPovorota = radians(PovorotGradusy);            // поворот в радианах (yaw)
    float UgolNaklona = radians(NaklonGradusy);              // наклон в радианах (pitch)
                                                                                                //точка на сфере единичного радиуса, умноженная на Distanciya
    Pozitsiya = TochkaVzglyada + Vec3(std::cos(UgolNaklona) * std::sin(UgolPovorota),
                                      std::sin(UgolNaklona),
                                      std::cos(UgolNaklona) * std::cos(UgolPovorota)) *
                                     Distanciya;
  }

  Mat4 view() const { return Mat4::lookAt(Pozitsiya, TochkaVzglyada, VektorVverh); }                                       // матрица вида-мир->система камеры
  Mat4 projection() const {//матрица проекции-система камеры->clipпространство
    return Mat4::perspective(UgolObzoraGradusy, SootnoshenieStoron, BlizhnyayaPloskost, DalnyayaPloskost);
  }
};
