#ifndef HTMLUTILS_HH
#define HTMLUTILS_HH

#include "html.hh"

namespace html
{
  inline Html hTag(std::string name)
  {
    return Html().wName(std::move(name));
  }

  inline Html hText(std::string text)
  {
    return Html().wText(std::move(text));
  }

  inline Html::Attr hAttr(std::string name, std::string value)
  {
    return {std::move(name), std::move(value)};
  }

  inline Html::SelfClose hSelfClose() { return {}; }

  inline Html::NoClose hNoClose() { return {}; }

  inline Html hNop() { return {}; }

  inline Html::Dump hDump() { return {}; }
}

#endif
