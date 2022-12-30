#include "imgbrd.hh"

int main(int argc, char **argv)
{
  ImgBrd ib;
  ib.init(argc, argv);
  ib.exec();

  return 0;
}
