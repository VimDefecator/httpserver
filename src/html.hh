#ifndef HTML_HH
#define HTML_HH

#include <vector>
#include <string>
#include <memory>
#include "classutils.hh"

class Html : public EnableMoveGetter<Html>
{
public:
  Html();
  Html(std::string text);
  Html(std::string name, std::initializer_list<std::pair<std::string_view, std::string_view>> attrs);

  ~Html();

  Html(Html &&);
  Html &operator=(Html &&);

  Html &setText(std::string text);
  Html &setName(std::string name);
  Html &addAttr(std::string name, std::string value);
  Html &addChild(Html &&child);

  operator bool();

  std::string dump();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif
