#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "common/classutils.hh"

class Html : public EnableMoveGetter<Html>
{
public:
  Html();
  ~Html();

  Html(Html &&);
  Html &operator=(Html &&);

  Html(const Html &);
  Html &operator=(const Html &);

  void setRawHtml(std::string raw);
  void setText(std::string text);
  void setName(std::string name);
  void addAttr(std::string name, std::string value);
  void addChild(Html child);

  void apply(const std::function<void(Html&)> &fn) {
    fn(*this);
  }
  void applyIf(bool cond, const std::function<void(Html&)> &fn) {
    if(cond) fn(*this);
  }

  void selfClose();
  void noClose();
  void nop() {};

  std::string dump();

  CHAIN_METHOD_AUTO(wRaw, setRawHtml);
  CHAIN_METHOD_AUTO(wText, setText);
  CHAIN_METHOD_AUTO(wName, setName);
  CHAIN_METHOD_AUTO(wAttr, addAttr);
  CHAIN_METHOD_AUTO(wChild, addChild);
  CHAIN_METHOD_AUTO(wApply, apply);
  CHAIN_METHOD_AUTO(wApplyIf, applyIf);
  CHAIN_METHOD_AUTO(wSelfClose, selfClose);
  CHAIN_METHOD_AUTO(wNoClose, noClose);

  CHAIN_METHOD_FOR_TYPE_AUTO(operator<<, addChild, Html);

  CHAIN_METHOD_FOR_TYPE_AUTO(operator<<, apply, std::function<void(Html&)>);

  struct ApplyIf { bool cond; const std::function<void(Html&)> &fn; };
  void applyIf(ApplyIf condFn) { applyIf(condFn.cond, condFn.fn); }
  CHAIN_METHOD_FOR_TYPE_AUTO(operator<<, applyIf, ApplyIf);

  struct Attr { std::string name, value; };
  void addAttr(Attr attr) { addAttr(std::move(attr.name), std::move(attr.value)); }
  CHAIN_METHOD_FOR_TYPE_AUTO(operator<<, addAttr, Attr);

  struct RawHtml { std::string raw; };
  void setRawHtml(RawHtml rawHtml) { setRawHtml(std::move(rawHtml.raw)); }
  CHAIN_METHOD_FOR_TYPE_AUTO(operator<<, setRawHtml, RawHtml);

  CHAIN_METHOD_FOR_TYPE_NO_ARG_AUTO(operator<<, selfClose, SelfClose);
  CHAIN_METHOD_FOR_TYPE_NO_ARG_AUTO(operator<<, noClose, NoClose);
  CHAIN_METHOD_FOR_TYPE_NO_ARG_AUTO(operator<<, dump, Dump);
  CHAIN_METHOD_FOR_TYPE_NO_ARG_AUTO(operator<<, nop, Nop);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

