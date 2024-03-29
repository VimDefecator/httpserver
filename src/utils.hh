#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <optional>
#include <cstring>
#include <errno.h>
#include "common/str2num.hh"

template<class T>
void zfill(T *dst)
{
  memset(dst, 0, sizeof(T));
}

template<typename Ret>
Ret throwOnErr(Ret ret)
{
  if(ret == -1)
    throw std::runtime_error(strerror(errno));

  return ret;
}

template<class Res, typename Path>
std::optional<Res> getFileAs(Path&& path)
{
  if(auto file = std::ifstream(std::forward<Path>(path)))
    return {Res(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>())};
  else
    return {};
}
