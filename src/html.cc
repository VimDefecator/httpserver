#include "html.hh"
#include <iterator>
#include <sstream>

namespace
{
  void escapeStr(std::ostream &out, std::string_view str)
  {
    for(auto ch : str)
    {
      switch(ch)
      {
        case '<' : out << "&lt;"; break;
        case '>' : out << "&gt;"; break;
        case '&' : out << "&amp;"; break;
        case '"' : out << "&quot;"; break;
        case '\\' : out << "&bsol;"; break;
        default  : out << ch; break;
      }
    }
  }
}

struct Html::Impl
{
  enum class Type { Tag, Text };
  enum class TagType { Pair, SelfClose, NoClose };

  void dump(std::ostream &out, int indSize)
  {
    auto ind = std::string(indSize, ' ');

    if(type_ == Type::Tag)
    {
      out << ind << '<' << name_;
      for(auto &[name, value] : attribs_)
      {
        out << ' ' << name;
        if(!value.empty())
        {
          out << "=\"";
          escapeStr(out, value);
          out << '\"';
        }
      }
      
      if(tagType_ == TagType::Pair)
      {
        out << ">\n";

        for(auto &child : children_)
          child.impl_->dump(out, indSize + 2);

        out << ind << "</" << name_ << ">\n";
      }
      else if(tagType_ == TagType::SelfClose)
      {
        out << "/>\n";
      }
      else
      {
        out << ">\n";
      }
    }
    else
    {
      escapeStr(out, text_);
      out << '\n';
    }
  }

  Type type_;
  TagType tagType_ = TagType::Pair;

  std::string name_;
  std::vector<std::pair<std::string, std::string>> attribs_;
  std::vector<Html> children_;

  std::string text_;
};

Html::Html()
{
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

void Html::setText(std::string text)
{
  if(!impl_)
    impl_ = std::make_unique<Impl>();

  impl_->type_ = Impl::Type::Text;
  impl_->text_ = std::move(text);
}

void Html::setName(std::string name)
{
  if(!impl_)
    impl_ = std::make_unique<Impl>();

  impl_->type_ = Impl::Type::Tag;
  impl_->name_ = std::move(name);
}

void Html::addAttr(std::string name, std::string value)
{
  impl_->attribs_.emplace_back(std::move(name), std::move(value));
}

void Html::addChild(Html child)
{
  impl_->children_.push_back(std::move(child));
}

void Html::applyFn(std::function<void(Html&)> fn)
{
  fn(*this);
}

void Html::selfClose()
{
  impl_->tagType_ = Impl::TagType::SelfClose;
}

void Html::noClose()
{
  impl_->tagType_ = Impl::TagType::NoClose;
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
