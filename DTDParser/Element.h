#ifndef ARABICA_DTDPARSER_ELEMENT_H
#define ARABICA_DTDPARSER_ELEMENT_H

// $Id: Element.h,v 1.5 2001/12/06 16:25:56 jez Exp $

#include <vector>
#include <ostream>
#include "ContentModel.h"
#include "Attribute.h"

namespace DTD
{

template<class stringT>
class Element
{
  public:
    Element() { }
    Element(const Element& rhs);
    ~Element() { }
    Element& operator=(const Element& rhs);

    typedef std::vector<Attribute<stringT> > AttributeList;

    const stringT& name() const { return name_; }
    const ContentModel<stringT>& content_model() const { return content_model_; }
    const AttributeList& attributes() const { return attributes_; }

    void set_name(const stringT& name) { name_ = name; }
    void add_attribute(const Attribute<stringT>& attr) { attributes_.push_back(attr); }
    void set_content_model(const ContentModel<stringT>& model) { content_model_ = model; }

  private:
    stringT name_;
    ContentModel<stringT> content_model_;
    AttributeList attributes_;

    bool operator==(const Element&) const; // no impl
}; // class Element


template<class stringT>
Element<stringT>::Element(const Element<stringT>& rhs) :
   name_(rhs.name_),
  attributes_(rhs.attributes_.begin(), rhs.attributes_.end()),
  content_model_(rhs.content_model_)
{
} // Element

template<class stringT>
Element<stringT>& Element<stringT>::operator=(const Element<stringT>& rhs)
{
  if(&rhs == this)
    return *this;

  name_ = rhs.name_;
  content_model_ = rhs.content_model_;
  attributes_.clear();
  attributes_.insert(attributes_.begin(), rhs.attributes_.begin(), rhs.attributes_.end());

  return *this;
} // operator=

template<class stringT>
std::ostream& operator<<(std::ostream& os, const Element<stringT>& element)
{
  typedef Arabica::Unicode<stringT::value_type> UnicodeT;

  os << UnicodeT::LESS_THAN_SIGN << UnicodeT::EXCLAMATION_MARK << "ELEMENT" << UnicodeT::SPACE 
     << element.name() 
     << UnicodeT::SPACE 
     << element.content_model() 
     << UnicodeT::GREATER_THAN_SIGN
     << std::endl;

  if(element.attributes().size())
  {
    os << UnicodeT::LESS_THAN_SIGN << UnicodeT::EXCLAMATION_MARK << "ATTLIST" << UnicodeT::SPACE << element.name();
    for(Element<stringT>::AttributeList::const_iterator i = element.attributes().begin(); i != element.attributes().end(); ++i)
      os << std::endl << UnicodeT::SPACE << UnicodeT::SPACE << *i;
    os << UnicodeT::GREATER_THAN_SIGN << std::endl;
  } // if ...

  return os;
} // operator<<

} // namespace DTD

#endif