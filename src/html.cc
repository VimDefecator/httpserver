#include "html.hh"
#include <iterator>
#include <sstream>
#include <iostream>

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
  enum class Type { Many, Tag, Text, Raw };
  enum class TagType { Pair, SelfClose, NoClose };

  void dump(std::ostream &out)
  {
    if(type_ == Type::Many)
    {
      for(auto &child : children_)
        child.impl_->dump(out);
    }
    else if(type_ == Type::Tag)
    {
      out << '<' << str_;
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
        out << '>';

        if(!children_.empty())
        {
          out << '\n';

          for(auto &child : children_)
            child.impl_->dump(out);
        }

        out << "</" << str_ << ">\n";
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
    else if(type_ == Type::Text)
    {
      escapeStr(out, str_);
      out << '\n';
    }
    else
    {
      out << str_ << '\n';
    }
  }

  Type type_ = Type::Many;
  TagType tagType_ = TagType::Pair;

  std::string str_;
  std::vector<std::pair<std::string, std::string>> attribs_;
  std::vector<Html> children_;
};

Html::Html()
{
  impl_ = std::make_unique<Impl>();
}

Html::~Html()
{
}

Html::Html(Html &&other)
{
  impl_ = std::move(other.impl_);
}

Html &Html::operator=(Html &&other)
{
  impl_ = std::move(other.impl_);
  return *this;
}

Html::Html(const Html &other)
{
  impl_ = std::make_unique<Impl>(*other.impl_);
}

Html &Html::operator=(const Html &other)
{
  *impl_ = *other.impl_;
  return *this;
}

void Html::setRawHtml(std::string raw)
{
  impl_->type_ = Impl::Type::Raw;
  impl_->str_ = std::move(raw);
}

void Html::setText(std::string text)
{
  impl_->type_ = Impl::Type::Text;
  impl_->str_ = std::move(text);
}

void Html::setName(std::string name)
{
  impl_->type_ = Impl::Type::Tag;
  impl_->str_ = std::move(name);
}

void Html::addAttr(std::string name, std::string value)
{
  impl_->attribs_.emplace_back(std::move(name), std::move(value));
}

void Html::addChild(Html child)
{
  impl_->children_.emplace_back(std::move(child));
}

void Html::selfClose()
{
  impl_->tagType_ = Impl::TagType::SelfClose;
}

void Html::noClose()
{
  impl_->tagType_ = Impl::TagType::NoClose;
}

std::string Html::dump()
{
  std::stringstream ss;
  ss << "<!DOCTYPE html>\n";
  impl_->dump(ss);
  return ss.str();
}
