#ifndef HTML_HH
#define HTML_HH

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "classutils.hh"

class Html
{
public:
  Html();
  ~Html();

  Html(Html &&);
  Html &operator=(Html &&);

  Html(const Html &);
  Html &operator=(const Html &);

  void setText(std::string text);
  void setName(std::string name);
  void addAttr(std::string name, std::string value);
  void addChild(Html child);

  void applyFn(std::function<void(Html&)> fn);

  void selfClose();
  void noClose();

  CHAINMETHOD(wText, setText);
  CHAINMETHOD(wName, setName);
  CHAINMETHOD(wAttr, addAttr);
  CHAINMETHOD(wChild, addChild);
  CHAINMETHOD(operator<<, addChild);
  CHAINMETHOD(wFunc, applyFn);
  CHAINMETHOD(wSelfClose, selfClose);
  CHAINMETHOD(wNoClose, noClose);

  struct Dump {};
  std::string operator<<(Dump) { return dump(); }

  operator bool();

  std::string dump();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

inline Html hTag(std::string name)
{
  return Html().wName(std::move(name));
}

inline Html hText(std::string text)
{
  return Html().wText(std::move(text));
}

#endif
