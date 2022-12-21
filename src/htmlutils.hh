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

  inline Html::Dump hDump() { return {}; }

  inline Html hFieldSet(std::string legend,
                        std::initializer_list<std::pair<std::string_view, std::string_view>> attrs,
                        std::initializer_list<std::string_view> labels)
  {
    return
      hTag("fieldset")
      << (hTag("legend")
        << hText(std::move(legend)))
      << (hTag("p")
        << [&](auto &h){
          for(auto label : labels)
            h << (hTag("label")
                << (hTag("input")
                  << [&](auto &h){
                    for(auto [name, value] : attrs)
                      h << hAttr(std::string(name), std::string(value));}
                  << hNoClose())
                << hText(std::string(label)));});
  }
}

#endif
