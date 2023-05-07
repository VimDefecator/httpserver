#pragma once

#include <memory>

class Args;

class ImgBrd
{
public:
  ImgBrd();
  ~ImgBrd();

  void init(const Args &args);
  void exec();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

