// model.cpp: загрузка модели через Assimp и копирование данных в наши массивы.
#include "model.h"
#include <assimp/postprocess.h> 
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

bool Model::load(const std::string& PutKFajlu) {
  Assimp::Importer ZagruzchikAssimp;

  // aiProcess_Triangulate: разрезает многоуг на треуг (растеризатору)
  const aiScene* Scena = ZagruzchikAssimp.ReadFile(
      PutKFajlu, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
  if (!Scena || !Scena->mRootNode) {
    return false;  // файл не прочитался
  }
  // В одном файле может быть несколько сеток (mMeshes), проходим по всем
  for (unsigned int NomerSetki = 0; NomerSetki < Scena->mNumMeshes;                                                                             // Обработка мешей (сеток)
       NomerSetki++) {
    const aiMesh* Setka = Scena->mMeshes[NomerSetki];
    int SmeshenieVershin =  // чтобы индексы вершин в общем массиве не перезаписывали друг друга
        (int)Vershiny_.size();
    int SmeshenieNormalej =
        (int)Normali_.size();
    int SmeshenieTekstur =
        (int)Tekstury_.size();

    // Копируем вершины и нормали                                                                                                                 Чтение вершин и атрибутов
    for (unsigned int NomerVershiny = 0; NomerVershiny < Setka->mNumVertices;
         NomerVershiny++) {
      Vershiny_.push_back(Vec3(Setka->mVertices[NomerVershiny].x,
                               Setka->mVertices[NomerVershiny].y,
                               Setka->mVertices[NomerVershiny].z));
      if (Setka->HasNormals()) {  //если нормали есть в файле
        Normali_.push_back(Vec3(Setka->mNormals[NomerVershiny].x,
                                Setka->mNormals[NomerVershiny].y,
                                Setka->mNormals[NomerVershiny].z));
      } else {
        Normali_.push_back(
            Vec3(0.f, 0.f,
                 1.f));  // нормалей нет: ставим любую, чтобы номера не сбились
      }

      // копируем текстурные координаты (u, v).
      if (Setka->HasTextureCoords(0)) {  //берем первый набор UV(0)
        Tekstury_.push_back(Vec2(Setka->mTextureCoords[0][NomerVershiny].x,
                                 Setka->mTextureCoords[0][NomerVershiny].y));
      } else {
        Tekstury_.push_back(
            Vec2(0.f, 0.f));  //нет текстуh коорд:ставим(0,0),чтобы номера не сбились
      }
    }

    // Копируем треугольники
    for (unsigned int NomerGrani = 0; NomerGrani < Setka->mNumFaces;
         NomerGrani++) {
      const aiFace& Gran = Setka->mFaces[NomerGrani];  // текущая грань
      if (Gran.mNumIndices != 3) {
        continue;  // нужны только треугольники
      }
      Face NovyjTreugolnik;  // наш треугольник
      for (int NomerUgla = 0; NomerUgla < 3; NomerUgla++) {
        int IndeksVershiny =
            Gran.mIndices[NomerUgla];  // номер вершины внутри этой сетки
        NovyjTreugolnik.NomeraVershin[NomerUgla] =
            SmeshenieVershin + IndeksVershiny;  // номер в общем массиве вершин
        NovyjTreugolnik.NomeraNormalej[NomerUgla] =
            SmeshenieNormalej +
            IndeksVershiny;  // номер в общем массиве нормалей
        NovyjTreugolnik.NomeraTekstur[NomerUgla] =
            SmeshenieTekstur +
            IndeksVershiny;  //номер в общем массиве текстурных координат
      }
      Treugolniki_.push_back(NovyjTreugolnik);
    }
  }

  if (Vershiny_.empty() || Treugolniki_.empty()) {                                                                                                          // Матрица модели
    return false;
  }

  // Ищем габариты модели: самые маленькие и самые большие x, y, z
  GabaritMin = Vershiny_[0];
  GabaritMax = Vershiny_[0];
  for (int Nomer = 1; Nomer < (int)Vershiny_.size(); Nomer++) {
    Vec3 Tochka = Vershiny_[Nomer];
    if (Tochka.x < GabaritMin.x) GabaritMin.x = Tochka.x;
    if (Tochka.y < GabaritMin.y) GabaritMin.y = Tochka.y;
    if (Tochka.z < GabaritMin.z) GabaritMin.z = Tochka.z;
    if (Tochka.x > GabaritMax.x) GabaritMax.x = Tochka.x;
    if (Tochka.y > GabaritMax.y) GabaritMax.y = Tochka.y;
    if (Tochka.z > GabaritMax.z) GabaritMax.z = Tochka.z;
  }
  return true;
}
