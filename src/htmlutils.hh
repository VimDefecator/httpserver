#pragma once

#include "html.hh"

namespace html
{
  inline Html hMany()
  {
    return Html();
  }

  inline Html hTag(std::string name)
  {
    return Html().wName(std::move(name));
  }

  inline Html hText(std::string text)
  {
    return Html().wText(std::move(text));
  }

  inline Html hRaw(std::string raw)
  {
    return Html().wRaw(std::move(raw));
  }

  inline Html::Attr hAttr(std::string name, std::string value)
  {
    return {std::move(name), std::move(value)};
  }

  inline Html::ApplyIf hIf(bool cond, const std::function<void(Html&)> &fn)
  {
    return {cond, fn};
  }

  inline Html::SelfClose hSelfClose() { return {}; }
  inline Html::NoClose hNoClose() { return {}; }
  inline Html::Dump hDump() { return {}; }
  inline Html::Nop hNop() { return {}; }
}

