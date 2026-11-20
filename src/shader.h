#pragma once
#include <tgaimage.h>

#include <cmath>

#include "geometry.h"
#include "model.h"

// Базовый контракт шейдера для растеризатора: расчёт вершины и фрагмента
struct IShader {
  virtual ~IShader() {}  // виртуальный деструктор
    // Возвращает позицию вершины в clip-пространстве для заданного треугольника и угла
  virtual Vec4 vertex(int NomerTreygolnika, int NomerUgla) = 0;
  // Фрагментный шейдер: вызывается для каждого пикселя треугольника.
  // Вычисляет цвет пикселя по барицентрическим весам; true — отбросить фрагмент
  virtual bool fragment(const Vec3& Baricentr, TGAColor& Tsvet) = 0;
};

// Переводит число из диапазона 0..1 в число из диапазона 0..255
inline unsigned char VKanal(float Znachenie) {
  if (Znachenie < 0.f) {  // меньше нуля нельзя
    Znachenie = 0.f;
  }
  if (Znachenie > 1.f) {  // больше единицы нельзя
    Znachenie = 1.f;
  }
  return (unsigned char)(Znachenie * 255.f);
}

// Шейдер затенения по Гуро с моделью освещения Фонга в вершинах
struct GouraudShader : IShader {
  //Входные данные сцены (uniforms), задаваемые в main.cpp                                                                                          Настройка шейдера (Uniforms)
  const Model* UkazatelNaModel = nullptr;                                    //какая модель рисуется
  Mat4 MatricaModeli = Mat4::identity();   //центр и масштаб
  Mat4 MatricaVidProekcii = Mat4::identity();  //Проекция * Вид
  Vec3 NapravlenieSveta = Vec3(0, 0, 1);  //единич.вект на источник света
  Vec3 PozitsiyaKamery = Vec3(0, 0, 3);   //нужна для блика

  const TGAImage* UkazatelNaTekstury = nullptr;

  // Коэффициенты фонового и диффузного освещения                                                                                                     // Фонг: Ambient + Diffuse
  float KoefFonovogo = 0.10f;    //фоновый свет
  float KoefDiffuznogo = 0.80f;  //матовое освещение
  //резкость пятна блика                                                                                                                               // Фонг: Specular (Блики)
  float KoefBlika = 0.50f;
  float Blesk = 32.f;

  // Белый базовый цвет, чтобы не затемнять текстуру повторно при умножении
  Vec3 BazovyjTsvetPoverhnosti = Vec3(1.f, 1.f, 1.f);

  // Вычисленные в вершинах цвета для последующей интерполяции во фрагментном шейдере
  Vec3 TsvetaVershin[3];
  // Текстурные координаты (u, v) трёх вершин треугольника                                                                                                      // Передача UV
  Vec2 TeksturnyeKoordinatyVershin[3];

  //мировые координаты, нормаль, свет Фонга и передача атрибутов                                                                                               Вершинный шейдер
  Vec4 vertex(int NomerTreygolnika, int NomerUgla) override {
     // Перевод позиции вершины в мировое пространство (w = 1 для точки)
      Vec4 MirovayaPozitsiya =
        MatricaModeli *
        Vec4(UkazatelNaModel->vert(NomerTreygolnika, NomerUgla), 1.f);
    Vec3 Pozitsiya = MirovayaPozitsiya.xyz();

    // Преобразование нормали в мир.простр.нормализация w=0 для вект
    Vec3 Normal = normalize(
        (MatricaModeli *
         Vec4(UkazatelNaModel->normal(NomerTreygolnika, NomerUgla), 0.f))
            .xyz());

    Vec3 NapravlenieNaKameru = normalize(PozitsiyaKamery - Pozitsiya);

    // Диффузный вклад по закону Ламберта: max(0, N·L)                                                                                               // Фонг: Ambient + Diffuse
    float Diffuznaya = dot(Normal, NapravlenieSveta);                                                                                                //dot дает кос.угла.между нормалью и светом
    if (Diffuznaya < 0.f) {  // свет падает с обратной стороны: света нет
      Diffuznaya = 0.f;
    }

    // Зеркальный блик по отражен лучу R и направлению на камеру                                                                                      // Фонг: Specular (Блики)
    float Blik = 0.f;
    if (Diffuznaya > 0.f) {  //блик только на свету
      Vec3 OtrazhennyjLuch =
          reflect(NapravlenieSveta, Normal);  //куда отразился свет
      float Kosinus = dot(OtrazhennyjLuch, NapravlenieNaKameru);
      if (Kosinus < 0.f) {
        Kosinus = 0.f;
      }
      Blik = KoefBlika * std::pow(Kosinus, Blesk);
    }
    // Итоговый цвет вершины: ambient + diffuse + specular
    TsvetaVershin[NomerUgla] =
        BazovyjTsvetPoverhnosti * (KoefFonovogo + KoefDiffuznogo * Diffuznaya) +       //формула
        Vec3(Blik, Blik, Blik);
    // Сохранение UV-координат вершины для интерполяции в пикселях                                                                                              //Передача UV
    TeksturnyeKoordinatyVershin[NomerUgla] =
        UkazatelNaModel->TeksturnyeKoordinaty(NomerTreygolnika, NomerUgla);

    return MatricaVidProekcii * MirovayaPozitsiya;
  }

  //интерполяция цвета вершин,текстурирование и запись в пиксель                                                                                     //Фрагментный шейдер (Гуро)
  bool fragment(const Vec3& Baricentr, TGAColor& Tsvet) override {
    Vec3 ItogovyjTsvet = TsvetaVershin[0] * Baricentr.x + //лин.интерполяция.цветов вершин по барицентрическим координатам
                         TsvetaVershin[1] * Baricentr.y +
                         TsvetaVershin[2] * Baricentr.z;

    Vec3 TsvetIzTekstury = Vec3(0.7f, 0.7f, 0.7f);

    if (UkazatelNaTekstury != nullptr) {                                                                                                            // Интерполяция UV (Текстура)
      float KoordinataU = TeksturnyeKoordinatyVershin[0].x * Baricentr.x + // Интерполяция u,v внутри треуг
                          TeksturnyeKoordinatyVershin[1].x * Baricentr.y +
                          TeksturnyeKoordinatyVershin[2].x * Baricentr.z;
      float KoordinataV = TeksturnyeKoordinatyVershin[0].y * Baricentr.x +
                          TeksturnyeKoordinatyVershin[1].y * Baricentr.y +
                          TeksturnyeKoordinatyVershin[2].y * Baricentr.z;

      KoordinataV = 1.f - KoordinataV;

      // Ограничение UV-координат диапазоном0,1
      if (KoordinataU < 0.f) {
        KoordinataU = 0.f;
      }
      if (KoordinataU > 1.f) {
        KoordinataU = 1.f;
      }
      if (KoordinataV < 0.f) {
        KoordinataV = 0.f;
      }
      if (KoordinataV > 1.f) {
        KoordinataV = 1.f;
      }

      //переводим uv из диапаз 0..1 в номер пикселя текстуры
      int TeksturaX = (int)(KoordinataU * (UkazatelNaTekstury->width() - 1));
      int TeksturaY = (int)(KoordinataV * (UkazatelNaTekstury->height() - 1));

      TGAColor PikselTeksturi = UkazatelNaTekstury->get(TeksturaX, TeksturaY);

      TsvetIzTekstury.x = PikselTeksturi.bgra[2] / 255.f;
      TsvetIzTekstury.y = PikselTeksturi.bgra[1] / 255.f;
      TsvetIzTekstury.z = PikselTeksturi.bgra[0] / 255.f;
    }

    // Умножение интерполированного освещения на цвет текстуры
    ItogovyjTsvet.x = ItogovyjTsvet.x * TsvetIzTekstury.x;
    ItogovyjTsvet.y = ItogovyjTsvet.y * TsvetIzTekstury.y;
    ItogovyjTsvet.z = ItogovyjTsvet.z * TsvetIzTekstury.z;

    // Запись итогового цвета в формате BGRA
    Tsvet.bgra[0] = VKanal(ItogovyjTsvet.z);
    Tsvet.bgra[1] = VKanal(ItogovyjTsvet.y);
    Tsvet.bgra[2] = VKanal(ItogovyjTsvet.x);
    Tsvet.bgra[3] = 255;
    Tsvet.bytespp = 4;

    return false;
  }
};
