#ifndef ARABICA_DTDPARSER_PARAMETER_ENTITY_H
#define ARABICA_DTDPARSER_PARAMETER_ENTITY_H

// $Id: ParameterEntity.h,v 1.2 2001/12/13 16:11:38 jez Exp $

#include <ostream>

namespace DTD
{

template<class stringT>
class ParameterEntity
{
  public:
    ParameterEntity() { }
    ParameterEntity(const ParameterEntity& rhs);
    ~ParameterEntity() { }
    ParameterEntity& operator=(const ParameterEntity& rhs);

    const stringT& name() const { return name_; }
    const stringT& value() const { return value_; }

    void set_name(const stringT& name) { name_ = name; }
    void set_value(const stringT& value) { value_ = value; }

  private:
    stringT name_;
    stringT value_;

    bool operator==(const ParameterEntity&) const; // no impl
}; // class ParameterEntity


template<class stringT>
ParameterEntity<stringT>::ParameterEntity(const ParameterEntity<stringT>& rhs) :
   name_(rhs.name_),
   value_(rhs.value_)
{
} // ParameterEntity

template<class stringT>
ParameterEntity<stringT>& ParameterEntity<stringT>::operator=(const ParameterEntity<stringT>& rhs)
{
  if(&rhs == this)
    return *this;

  name_ = rhs.name_;
  value_ = rhs.value_;

  return *this;
} // operator=

} // namespace DTD

#endif