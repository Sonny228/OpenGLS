#pragma once
#include <string>
#include <vector>

#include "geometry.h"

class Model {
 public:
  bool load(const std::string& PutKFajlu);                                                                                                               //загружаем модель
  // Возвращ общее кол-во треуг в модели
  int nfaces() const {
    return (int)Treugolniki_.size();
  }

  // Позиция вершины у треугольника NomerTreygolnika берем NomerUgla
  Vec3 vert(int NomerTreygolnika, int NomerUgla) const {
    return Vershiny_[Treugolniki_[NomerTreygolnika].NomeraVershin[NomerUgla]];
  }
  // Возвращ нормаль конкретной вершины (угла) треугольника
  Vec3 normal(int NomerTreygolnika, int NomerUgla) const {
    return Normali_[Treugolniki_[NomerTreygolnika].NomeraNormalej[NomerUgla]];
  }

  //Возвращ текстур коорд(u,v)конкретной верш
  Vec2 TeksturnyeKoordinaty(int NomerTreygolnika, int NomerUgla) const {
    return Tekstury_[Treugolniki_[NomerTreygolnika].NomeraTekstur[NomerUgla]];
  }
  // Мин и мак коорд, для центрирования и масштабирования
  Vec3 GabaritMin, GabaritMax;                                                                                                                           // Матрица модели

 private:
  struct Face {    //Структура одного треугольника: храним индексы для каждой из 3 вершин
    int NomeraVershin[3], NomeraNormalej[3];
    int NomeraTekstur[3];                                                                                                                       // индексы для UV-координат
  };
  std::vector<Vec3> Vershiny_, Normali_;  // все вершины и все нормали модели
  std::vector<Vec2> Tekstury_;  //все текстурные координаты
  std::vector<Face> Treugolniki_;  // все треугольники
};
