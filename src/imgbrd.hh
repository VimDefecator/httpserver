#ifndef IMGBRD_HH
#define IMGBRD_HH

#include <memory>

class ImgBrd
{
public:
  ImgBrd();
  ~ImgBrd();

  void init(int argc = 0, char **argv = nullptr);
  void exec();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif
