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
  Html(std::string text);
  Html(std::string name, std::initializer_list<std::pair<std::string_view, std::string_view>> attrs);

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

  CHAINMETHOD(withText, setText);
  CHAINMETHOD(withName, setName);
  CHAINMETHOD(withAttr, addAttr);
  CHAINMETHOD(withChild, addChild);
  CHAINMETHOD(withAppliedFn, applyFn);
  CHAINMETHOD(withSelfClose, selfClose);
  CHAINMETHOD(withNoClose, noClose);

  operator bool();

  std::string dump();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif
