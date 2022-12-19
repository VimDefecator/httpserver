#include "html.hh"
#include <iterator>
#include <sstream>

struct Html::Impl
{
  enum class Type { C, L };

  void dump(std::ostream &out, int indSize)
  {
    auto ind = std::string(indSize, ' ');

    if(type_ == Type::C)
    {
      out << ind << '<' << name_;
      for(auto &[name, value] : attribs_)
      {
        out << ' ' << name << "=\"";
        for(auto ch : value)
        {
          if(ch == '\\' || ch == '\"') out << '\\';
          out << ch;
        }
        out << '\"';
      }
      
      if(!children_.empty())
      {
        out << ">\n";

        for(auto &child : children_)
          child.impl_->dump(out, indSize + 2);

        out << ind << "</" << name_ << ">\n";
      }
      else
      {
        out << " />\n";
      }
    }
    else
    {
      out << ind << text_ << '\n';
    }
  }

  Type type_;

  std::string name_;
  std::vector<std::pair<std::string, std::string>> attribs_;
  std::vector<Html> children_;

  std::string text_;
};

Html::Html()
{
}

Html::Html(std::string text)
{
  setText(std::move(text));
}

Html::Html(std::string name, std::initializer_list<std::pair<std::string_view, std::string_view>> attrs)
{
  setName(std::move(name));
  for(auto [name, value] : attrs)
    addAttr(std::string(name), std::string(value));
}

Html::~Html()
{
}

Html::Html(Html &&other)
{
  *this = std::move(other);
}

Html &Html::operator=(Html &&other)
{
  impl_ = std::move(other.impl_);
  return *this;
}

Html::Html(const Html &other)
{
  *this = other;
}

Html &Html::operator=(const Html &other)
{
  if(!impl_)
    impl_ = std::make_unique<Impl>();

  *impl_ = *other.impl_;

  return *this;
}

Html &Html::setText(std::string text)
{
  if(!impl_)
    impl_ = std::make_unique<Impl>();

  impl_->type_ = Impl::Type::L;
  impl_->text_ = std::move(text);

  return *this;
}

Html &Html::setName(std::string name)
{
  if(!impl_)
    impl_ = std::make_unique<Impl>();

  impl_->type_ = Impl::Type::C;
  impl_->name_ = std::move(name);

  return *this;
}

Html &Html::addAttr(std::string name, std::string value)
{
  impl_->attribs_.emplace_back(std::move(name), std::move(value));
  return *this;
}

Html &Html::addChild(Html &&child)
{
  impl_->children_.push_back(std::move(child));
  return *this;
}

Html &Html::addChild(const Html &child)
{
  impl_->children_.push_back(child);
  return *this;
}

Html &Html::applyFn(std::function<void(Html&)> fn)
{
  fn(*this);
  return *this;
}

Html::operator bool()
{
  return bool(impl_);
}

std::string Html::dump()
{
  std::stringstream ss;
  ss << "<!DOCTYPE html>\n";
  impl_->dump(ss, 0);
  return ss.str();
}
