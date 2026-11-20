#pragma once
#include <cmath>

constexpr float ChisloPi = 3.14159265358979f;                                                                                    //переводить градусы в радианы
inline float radians(float Gradusy) { return Gradusy * ChisloPi / 180.f; }                                                              // перевод из градусов в радианы (sin/cos ждут радианы)

//Двумерный вектор (для UV-координат)
struct Vec2 {                                   // используем для текстурных координат (u, v)
  float x = 0, y = 0;                           // две компоненты, по умолчанию нули
  Vec2() = default;                             // конструктор по умолчанию: вектор (0, 0)
  Vec2(float ZnachenieX, float ZnachenieY) : x(ZnachenieX), y(ZnachenieY) {}  // конструктор из двух чисел
};

//Трехмерный вектор (точка или направление)
struct Vec3 {                                   // точка или направление в пространстве
  float x = 0, y = 0, z = 0;                    // три компоненты, по умолчанию нули
  Vec3() = default;                             // конструктор по умолчанию: вектор (0, 0, 0)
  Vec3(float ZnachenieX, float ZnachenieY, float ZnachenieZ) : x(ZnachenieX), y(ZnachenieY), z(ZnachenieZ) {}  // из трех чисел
};

// Четырехмерный вектор (однородные координаты)
struct Vec4 {                                                                               //w нужна чтобы перенос и перспектива стали матрицами
  float x = 0, y = 0, z = 0, w = 0;
  Vec4() = default;                                                                     // констр по умолч.все нули
  Vec4(float ZnachenieX, float ZnachenieY, float ZnachenieZ, float ZnachenieW) : x(ZnachenieX), y(ZnachenieY), z(ZnachenieZ), w(ZnachenieW) {}  // из четырёх чисел
  Vec4(const Vec3& Vektor3, float ZnachenieW) : x(Vektor3.x), y(Vektor3.y), z(Vektor3.z), w(ZnachenieW) {} //vec3 и w точка имеет w=1, направ имеет w=0
  Vec3 xyz() const { return Vec3(x, y, z); }                                                                // отбросить w и получить обычный vec3
};

//Базовые операции над векторами
inline Vec3 operator+(const Vec3& Levyj, const Vec3& Pravyj) {  // сумма векторов покомпонентно
  return {Levyj.x + Pravyj.x, Levyj.y + Pravyj.y, Levyj.z + Pravyj.z};
}
inline Vec3 operator-(const Vec3& Levyj, const Vec3& Pravyj) {  // разность векторов покомпонентно
  return {Levyj.x - Pravyj.x, Levyj.y - Pravyj.y, Levyj.z - Pravyj.z};
}
inline Vec3 operator*(const Vec3& Vektor, float Chislo) {       // умножение вектора на число (растягивает вектор)                            вектор * число
  return {Vektor.x * Chislo, Vektor.y * Chislo, Vektor.z * Chislo};
}
inline Vec3 operator*(float Chislo, const Vec3& Vektor) { return Vektor * Chislo; }  // то же самое, но число записано слева                  число * вектор

// Скаляр.произвед=кос.угла между век                                                                                                                          //ФОНГ-cos угла между единич.векторами
inline float dot(const Vec3& Pervyj, const Vec3& Vtoroj) {                                                                                   //Математика:dot,cross,normalize
  return Pervyj.x * Vtoroj.x + Pervyj.y * Vtoroj.y + Pervyj.z * Vtoroj.z;                                                                    //Используется в Фонг:Ambient+Diffuse(N·L)и бликах
}

//Вектор.произвед=перпендикулярный век
inline Vec3 cross(const Vec3& Pervyj, const Vec3& Vtoroj) {
  return {Pervyj.y * Vtoroj.z - Pervyj.z * Vtoroj.y,   
          Pervyj.z * Vtoroj.x - Pervyj.x * Vtoroj.z,   
          Pervyj.x * Vtoroj.y - Pervyj.y * Vtoroj.x};  
}
inline float length(const Vec3& Vektor) { return std::sqrt(dot(Vektor, Vektor)); }//корень из скалярного квадрата
inline Vec3 normalize(const Vec3& Vektor) {// сделать длину вектора равной 1 (направление сохраняется)
  float Dlina = length(Vektor);                                                                                             // текущая длина вектора
  return Dlina > 1e-12f ? Vektor * (1.f / Dlina) : Vec3(0, 0, 0);                                                   // делим на длину; от деления на ноль защищаемся
}
// Отражение.padayushchijLuch
//Часть луча вдоль нормали разворач,часть вдоль поверх.ост
inline Vec3 reflect(const Vec3& PadayushchijLuch, const Vec3& Normal) {                                                                                                //Фонг: Specular (Блики)
  return Normal * (2.f * dot(Normal, PadayushchijLuch)) - PadayushchijLuch;
} //если отражен.луч.попадает в камер.пиксель получает яркий блик.

struct Mat4 {
  float Elementy[4][4];                                                                                                 //[строка][столбец]
  //Единичная матрица
  static Mat4 identity() {
    Mat4 Rezultat{};
    for (int Nomer = 0; Nomer < 4; Nomer++) Rezultat.Elementy[Nomer][Nomer] = 1.f;                                                                              // единицы на главной диагонали
    return Rezultat;
  }
  static Mat4 translate(const Vec3& Sdvig) {   // матрица переноса
    Mat4 Rezultat = identity();
    Rezultat.Elementy[0][3] = Sdvig.x;
    Rezultat.Elementy[1][3] = Sdvig.y;
    Rezultat.Elementy[2][3] = Sdvig.z;
    return Rezultat;
  }
  //Матрица масштабирования
  static Mat4 scale(float Masshtab) {           //равномерное сжатие/растяжение
    Mat4 Rezultat = identity();
    Rezultat.Elementy[0][0] = Rezultat.Elementy[1][1] = Rezultat.Elementy[2][2] = Masshtab;  // растягиваем по x, y и z
    return Rezultat;
  }

  //перевод мир в систему координат камеры                                                                                                                                  Матрица.вида(lookAt)
  static Mat4 lookAt(const Vec3& PozitsiyaKamery, const Vec3& TochkaVzglyada, const Vec3& VektorVverh) {
    Vec3 OsZ = normalize(PozitsiyaKamery - TochkaVzglyada); //Z от цели к камере (взгляд вдоль -Z)
    Vec3 OsX = normalize(cross(VektorVverh, OsZ));       //X вправо
    Vec3 OsY = cross(OsZ, OsX);                          //Y вверх
    Mat4 Rezultat = identity();
    Rezultat.Elementy[0][0] = OsX.x;
    Rezultat.Elementy[0][1] = OsX.y;
    Rezultat.Elementy[0][2] = OsX.z;
    Rezultat.Elementy[0][3] = -dot(OsX, PozitsiyaKamery);                                //переносим камеру в начало координат
    Rezultat.Elementy[1][0] = OsY.x;
    Rezultat.Elementy[1][1] = OsY.y;
    Rezultat.Elementy[1][2] = OsY.z;
    Rezultat.Elementy[1][3] = -dot(OsY, PozitsiyaKamery);
    Rezultat.Elementy[2][0] = OsZ.x;
    Rezultat.Elementy[2][1] = OsZ.y;
    Rezultat.Elementy[2][2] = OsZ.z;
    Rezultat.Elementy[2][3] = -dot(OsZ, PozitsiyaKamery);                                                               //dot-считает кос.угла; сам не рисует, но шейдер использует его для света
    return Rezultat;
  }

  // Матрица проекции (сист.кам->clip-пространство)
  static Mat4 perspective(float UgolObzoraGradusy, float SootnoshenieStoron, float BlizhnyayaPloskost, float DalnyayaPloskost) {                                    // Математика: perspective
    float Fokus = 1.f / std::tan(radians(UgolObzoraGradusy) / 2.f);//cotan-чем шире угол, тем меньше.Fokus
    Mat4 Rezultat{};                                                 
    Rezultat.Elementy[0][0] = Fokus / SootnoshenieStoron;
    Rezultat.Elementy[1][1] = Fokus; // масштаб.y
    Rezultat.Elementy[2][2] = (DalnyayaPloskost + BlizhnyayaPloskost) / (BlizhnyayaPloskost - DalnyayaPloskost); // глубина.z.множитель
    Rezultat.Elementy[2][3] = 2.f * DalnyayaPloskost * BlizhnyayaPloskost / (BlizhnyayaPloskost - DalnyayaPloskost); // глубина.z.смещение
    Rezultat.Elementy[3][2] = -1.f;//w=-z создание перспективы
    return Rezultat;
  }                                                                                                    //послеюумнож на вектор w=0·x+0·y+(-1)·z+0·w=-z_camera

  // Матрица для нормалей (обратная транспонированная)                                                                                                             // Математика: normalMatrix
  Mat4 normalMatrix() const {
    Vec3 Stroka0(Elementy[0][0], Elementy[0][1], Elementy[0][2]);                                                               // первая строка верхней 3x3
    Vec3 Stroka1(Elementy[1][0], Elementy[1][1], Elementy[1][2]);
    Vec3 Stroka2(Elementy[2][0], Elementy[2][1], Elementy[2][2]);
    Vec3 Kofaktor0 = cross(Stroka1, Stroka2);// кофактор-из них собирается обратная матрица
    Vec3 Kofaktor1 = cross(Stroka2, Stroka0);
    Vec3 Kofaktor2 = cross(Stroka0, Stroka1);
    float Opredelitel = dot(Stroka0, Kofaktor0);//определитель матрицы
    float ObratnyjOpredelitel = std::fabs(Opredelitel) > 1e-12f ? 1.f / Opredelitel : 1.f;
    Mat4 MatricaNormalej = identity();
    const Vec3 Kofaktory[3] = {Kofaktor0, Kofaktor1, Kofaktor2};                                                                    // складываем кофакторы в массив(против дублирования.кода)
    for (int Nomer = 0; Nomer < 3; Nomer++) {                                       //по 3-строкам результата
      MatricaNormalej.Elementy[Nomer][0] = Kofaktory[Nomer].x * ObratnyjOpredelitel;
      MatricaNormalej.Elementy[Nomer][1] = Kofaktory[Nomer].y * ObratnyjOpredelitel;
      MatricaNormalej.Elementy[Nomer][2] = Kofaktory[Nomer].z * ObratnyjOpredelitel;
    }
    return MatricaNormalej;
  }
};

// Произведение двух матриц (Levaya * Pravaya)
inline Mat4 operator*(const Mat4& Levaya, const Mat4& Pravaya) {
  Mat4 Rezultat{};                              // от мусора.из памяти-чистый лист
  for (int Stroka = 0; Stroka < 4; Stroka++)    //строк.результата
    for (int Stolbec = 0; Stolbec < 4; Stolbec++)  //столб.результата
      for (int Sloj = 0; Sloj < 4; Sloj++)
        Rezultat.Elementy[Stroka][Stolbec] += Levaya.Elementy[Stroka][Sloj] * Pravaya.Elementy[Sloj][Stolbec];                                              // строка на столбец
  return Rezultat;
}
// Умножение матрицы на вектор-столбеЦ
inline Vec4 operator*(const Mat4& Matrica, const Vec4& Vektor) {
  return {
      Matrica.Elementy[0][0] * Vektor.x + Matrica.Elementy[0][1] * Vektor.y + Matrica.Elementy[0][2] * Vektor.z + Matrica.Elementy[0][3] * Vektor.w,
      Matrica.Elementy[1][0] * Vektor.x + Matrica.Elementy[1][1] * Vektor.y + Matrica.Elementy[1][2] * Vektor.z + Matrica.Elementy[1][3] * Vektor.w,
      Matrica.Elementy[2][0] * Vektor.x + Matrica.Elementy[2][1] * Vektor.y + Matrica.Elementy[2][2] * Vektor.z + Matrica.Elementy[2][3] * Vektor.w,
      Matrica.Elementy[3][0] * Vektor.x + Matrica.Elementy[3][1] * Vektor.y + Matrica.Elementy[3][2] * Vektor.z + Matrica.Elementy[3][3] * Vektor.w};   //получ w=-z
}
