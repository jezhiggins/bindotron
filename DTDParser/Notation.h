#ifndef ARABICA_DTDPARSER_NOTATION_H
#define ARABICA_DTDPARSER_NOTATION_H

// $Id: Notation.h,v 1.2 2001/12/10 08:41:44 jez Exp $

#include <ostream>

namespace DTD
{

template<class stringT>
class Notation
{
  public:
    Notation() { }
    Notation(const Notation& rhs);
    ~Notation() { }
    Notation& operator=(const Notation& rhs);

    const stringT& name() const { return name_; }
    const stringT& publicId() const { return publicId_; }
    const stringT& systemId() const { return systemId_; }

    void set_name(const stringT& name) { name_ = name; }
    void set_publicId(const stringT& publicId) { publicId_ = publicId; }
    void set_systemId(const stringT& systemId) { systemId_ = systemId; }

  private:
    stringT name_;
    stringT publicId_;
    stringT systemId_;

    bool operator==(const Notation&) const; // no impl
}; // class Notation


template<class stringT>
Notation<stringT>::Notation(const Notation<stringT>& rhs) :
   name_(rhs.name_),
   publicId_(rhs.publicId_),
   systemId_(rhs.systemId_)
{
} // Notation

template<class stringT>
Notation<stringT>& Notation<stringT>::operator=(const Notation<stringT>& rhs)
{
  if(&rhs == this)
    return *this;

  name_ = rhs.name_;
  publicId_ = rhs.publicId_;
  systemId_ = rhs.systemId_;

  return *this;
} // operator=


template<class stringT>
typename stringT::value_type find_quote(const stringT& str)
{
  typedef Arabica::Unicode<stringT::value_type> UnicodeT;
  if(str.find(UnicodeT::QUOTATION_MARK) != stringT::npos)
    return UnicodeT::APOSTROPHE;
  return UnicodeT::QUOTATION_MARK;
} // find_quote

template<class stringT>
std::ostream& operator<<(std::ostream& os, const Notation<stringT>& notation)
{
  typedef Arabica::Unicode<stringT::value_type> UnicodeT;

  os << UnicodeT::LESS_THAN_SIGN 
     << UnicodeT::EXCLAMATION_MARK 
     << "NOTATION" 
     << UnicodeT::SPACE
     << notation.name();

  if(!notation.publicId().empty())
  {
    stringT::value_type quot = find_quote(notation.publicId());
    os << " PUBLIC " << quot << notation.publicId() << quot;
    if(!notation.systemId().empty())
    {
      quot = find_quote(notation.systemId());
      os << UnicodeT::SPACE << quot << notation.systemId() << quot;
    } // ... 
  }
  else
  {
    stringT::value_type quot = find_quote(notation.systemId());
    os << " SYSTEM " << quot << notation.systemId() << quot;
  } // if ...

  os << UnicodeT::GREATER_THAN_SIGN << std::endl;
 
  return os;
} // operator<<

} // namespace DTD

#endif