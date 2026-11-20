#include <tgaimage.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "camera.h"
#include "model.h"
#include "rasterizer.h"
#include "shader.h"

int main(int argc,
         char** argv) {                                                                                                                //argc-число аргументов
  if (argc < 2) {                                                                                                                       //argv-сами аргументы
    std::cerr << "Укажите .obj файл\n";
    return 1;
  }

  std::string PutKModeli = argv[1];
  std::string PutVyhoda = "output.tga";
  std::string PutKTekstury = "";         //путь к текстуре
  int RazmerKartinki = 800; // картинка в пикселях
  float PovorotGradusy = 0.f; // поворот камеры вокруг модели
  float NaklonGradusy = 0.f;
  float Distanciya = 3.f;
  Vec3 VektorSveta(1.f, 1.f, 1.f);  // источник света

                                                                                                                                        //арг.комманд.строки
  // argv[0]-имя прог,argv[1]-путь к модели,с argv[2]: перв ключ -o, --yaw.
  for (int NomerArgumenta = 2; NomerArgumenta < argc;
       NomerArgumenta++) {  // ++ чтобы след.итерация не прочитала его как ключ
    std::string Argument = argv[NomerArgumenta];  // текущий ключ, например "-o"
    if (Argument == "-o" && NomerArgumenta + 1 < argc) {
      PutVyhoda = argv[++NomerArgumenta];  //-o - путь куда сохр.результат.
    } else if (Argument == "-t" && NomerArgumenta + 1 < argc) {
      PutKTekstury = argv[++NomerArgumenta];  // НОВОЕ: -t - путь к картинке-текстуре
    } else if (Argument == "-s" && NomerArgumenta + 1 < argc) {
      RazmerKartinki = std::atoi(argv[++NomerArgumenta]);  // размер картинки
    } else if (Argument == "--yaw" && NomerArgumenta + 1 < argc) {
      PovorotGradusy = (float)std::atof(argv[++NomerArgumenta]);  // поворот вокруг верт.оси
    } else if (Argument == "--pitch" && NomerArgumenta + 1 < argc) {
      NaklonGradusy = (float)std::atof(argv[++NomerArgumenta]);  // наклон
    } else if (Argument == "--dist" && NomerArgumenta + 1 < argc) {
      Distanciya = (float)std::atof(argv[++NomerArgumenta]);  // расстояние
    } else if (Argument == "--light" &&
               NomerArgumenta + 3 < argc) {  //направ.свет: три числа       atoi (целое) для -s; atof 45.5 (дробное) для углов, расстояний, света
      VektorSveta.x = (float)std::atof(argv[++NomerArgumenta]);
      VektorSveta.y = (float)std::atof(argv[++NomerArgumenta]);
      VektorSveta.z = (float)std::atof(argv[++NomerArgumenta]);
    }
  }
  if (RazmerKartinki < 16) { //защита от маленьких размеров
    RazmerKartinki = 16;
  }

                                                                                                                                            //Загрузка модели
  Model ZagruzhennayaModel;                    
  if (!ZagruzhennayaModel.load(PutKModeli)) {
    std::cerr << "Не удалось загрузить модель: " << PutKModeli << "\n";
    return 1;
  }

                                                                                                                                      //загрузка текстуры tga
  TGAImage Tekstura;  //загрузится картинка-текстура
  if (!PutKTekstury.empty()) {
    if (!Tekstura.read_tga_file(PutKTekstury)) {
      std::cerr << "Не удалось загрузить текстуру: " << PutKTekstury << "\n";
      return 1;
    }
  }

  //Нормализуем модель,центр в (0,0,0), размеры в кубе [-1, 1]                                                      чтоб камер,свет,глуб работали одинаково
  //Центр ограничивающего параллелепипеда—середина между min и max.                                            независ.от исход.размер и положения в файле
  Vec3 CentrModeli =
      (ZagruzhennayaModel.GabaritMin + ZagruzhennayaModel.GabaritMax) *
      0.5f;
  Vec3 GabaritModeli = ZagruzhennayaModel.GabaritMax -
                       ZagruzhennayaModel.GabaritMin;  // размеры модели по осям(шир,вы,глуб)
  
  float NaibolshayaStorona =
      GabaritModeli.x;  //ищем длин.сторону модели чтобы влезла в куб
  if (GabaritModeli.y > NaibolshayaStorona) {
    NaibolshayaStorona = GabaritModeli.y;
  }
  if (GabaritModeli.z > NaibolshayaStorona) {
    NaibolshayaStorona = GabaritModeli.z;
  }
  float PolovinaGabarita =
      NaibolshayaStorona * 0.5f;  //половина длин.стороны от -1 до +1

  //сдвиг центр модели в 0 (translate), потом уменьш (scale) делим коорлинат на эту велечину чтобы влезло в куб [-1,1]
  Mat4 MatricaModeli =
      Mat4::scale(1.f / PolovinaGabarita) * Mat4::translate(CentrModeli * -1.f); //

                                                                                                                                                    //Камера
  Camera Kamera;
  Kamera.SootnoshenieStoron = 1.f;  // картинка квадратная
  Kamera.orbit(PovorotGradusy, NaklonGradusy,
               Distanciya);  // ставим камеру вокруг модели

                                                                                                                //Настройка шейдера: передаем ему все нужное
  GouraudShader Shejder;
  Shejder.UkazatelNaModel = &ZagruzhennayaModel;
  Shejder.MatricaModeli = MatricaModeli;
  Shejder.MatricaVidProekcii =
      Kamera.projection() * Kamera.view();  //проекция * вид,
  Shejder.NapravlenieSveta =
      normalize(VektorSveta);  //направление света длина для блика                                              
  Shejder.PozitsiyaKamery =
      Kamera.Pozitsiya;  //позиция камеры для блика
  if (!PutKTekstury.empty()) {
    Shejder.UkazatelNaTekstury = &Tekstura;
  }

  //Буферы цвет и глубина
  TGAImage Kartinka(RazmerKartinki, RazmerKartinki,
                    TGAImage::RGB);

  // Цвет фона в TGAColor
  TGAColor TsvetFona;
  TsvetFona.bgra[0] = 0;
  TsvetFona.bgra[1] = 0;
  TsvetFona.bgra[2] = 0;
  TsvetFona.bgra[3] = 255;
  TsvetFona.bytespp = 4;

  for (int PikselY = 0; PikselY < RazmerKartinki;
       PikselY++) {
    for (int PikselX = 0; PikselX < RazmerKartinki; PikselX++) {
      Kartinka.set(PikselX, PikselY, TsvetFona);
    }
  }

                                                                                                                    //z-буффер глубин для кажд.пикселя храним расстояние до камеры
  std::vector<float> BufferGlubiny(RazmerKartinki * RazmerKartinki, 1000.f);

  //Вызов.растеризатора-рисуем все треугольники                                                                      //передаем.коорд.трех.верш и ссылку на шейдер
  for (int NomerTreygolnika = 0; NomerTreygolnika < ZagruzhennayaModel.nfaces();                                    //в треуг. Барицентр.появятся только внутри треуг
       NomerTreygolnika++) {                                                                                         //известны экран коорд пикс
    Vec4 VershinyClip[3];  // три вершины треугольника после вершинного шейдера (коорд экрана)
    for (int Vershina = 0; Vershina < 3; Vershina++) {
      VershinyClip[Vershina] =
          Shejder.vertex(NomerTreygolnika, Vershina);  // вершинный шейдер                                                  3D точка модели -> точка на экране.
    }
    triangle(VershinyClip, Shejder, Kartinka,                                                                                    ////z-буффер ПЕРЕДАЧА-по ссылке
             BufferGlubiny);  // растеризация + фрагментный шейдер                                     
  }

  //Сохранение картинки
  Kartinka.write_tga_file(PutVyhoda);  // путь из -o

  return 0;
}
