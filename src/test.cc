#include "imgbrd.hh"
#include "common/args.hh"

int main(int argc, char **argv)
{
  ImgBrd ib;
  ib.init(Args(argc, argv, {{"p", "port"}}, {}));
  ib.exec();

  return 0;
}
