#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <optional>
#include <cstring>
#include <errno.h>

template<class T>
void zfill(T *dst)
{
  memset(dst, 0, sizeof(T));
}

template<typename Ret>
Ret throwOnErr(Ret ret, std::string descr = "")
{
  if(ret == -1)
    throw std::runtime_error(
      (!descr.empty() ? "Failed to " + descr + ". Error: " : std::string()) + strerror(errno));

  return ret;
}

inline std::string_view interval2strview(const std::vector<char> &vec, size_t begPos, size_t endPos)
{
  return std::string_view(vec.data(), vec.size()).substr(begPos, endPos - begPos);
}

class AppendToCharVec
{
public:
  AppendToCharVec(std::vector<char> &vec)
    : vec_(vec)
  {
  }

  AppendToCharVec &app(char ch)
  {
    vec_.push_back(ch);
    return *this;
  }

  AppendToCharVec &app(std::string_view str)
  {
    vec_.insert(vec_.end(), str.begin(), str.end());
    return *this;
  }

  AppendToCharVec &app(const std::vector<char> &vec)
  {
    vec_.insert(vec_.end(), vec.begin(), vec.end());
    return *this;
  }

private:
  std::vector<char> &vec_;
};

template<class Res, typename Path>
std::optional<Res> getFileAs(Path&& path)
{
  if(auto file = std::ifstream(std::forward<Path>(path)))
    return {Res(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>())};
  else
    return {};
  
}

#include "str2num.hcc"
