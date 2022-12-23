#ifndef HTML_HH
#define HTML_HH

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "classutils.hh"

class Html : public EnableMoveGetter<Html>
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

  CHAIN_METHOD(wText, setText);
  CHAIN_METHOD(wName, setName);
  CHAIN_METHOD(wAttr, addAttr);
  CHAIN_METHOD(wChild, addChild);
  CHAIN_METHOD(wFunc, applyFn);
  CHAIN_METHOD(wSelfClose, selfClose);
  CHAIN_METHOD(wNoClose, noClose);

  CHAIN_METHOD_FOR_TYPE(operator<<, addChild, Html);
  CHAIN_METHOD_FOR_TYPE(operator<<, applyFn, std::function<void(Html&)>);

  struct Attr { std::string name, value; };
  void addAttr(Attr attr) { addAttr(std::move(attr.name), std::move(attr.value)); }
  CHAIN_METHOD_FOR_TYPE(operator<<, addAttr, Attr);

  CHAIN_METHOD_FOR_TYPE_NO_ARG(operator<<, selfClose, SelfClose);
  CHAIN_METHOD_FOR_TYPE_NO_ARG(operator<<, noClose, NoClose);

  struct Dump {};
  std::string operator<<(Dump) { return dump(); }

  std::string dump();

  operator bool();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif
