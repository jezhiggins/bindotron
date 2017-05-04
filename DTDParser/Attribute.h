#ifndef ARABICA_DTDPARSER_ATTRIBUTE_H
#define ARABICA_DTDPARSER_ATTRIBUTE_H

// $Id: Attribute.h,v 1.8 2001/12/11 14:41:14 jez Exp $

#include <string>
#include <vector>
#include <XML/UnicodeCharacters.h>

namespace DTD
{

template<class stringT>
class Attribute
{
  public:
    enum Type
    {
      CDATA,
      NMTOKEN,
      NMTOKENS,
      ENUMERATION,
      ENTITY,
      ENTITIES,
      ID,
      IDREF,
      IDREFS,
      NOTATION,
      UNKNOWN_TYPE
    };

    enum DefaultValue
    {
      IMPLIED,
      REQUIRED,
      FIXED,
      LITERAL,
      UNKNOWN_VALUE
    }; 

    typedef std::vector<stringT> EnumValueList;

    Attribute() : type_(UNKNOWN_TYPE), default_value_(UNKNOWN_VALUE) { }
    Attribute(const Attribute& rhs);
    ~Attribute() { }
    Attribute& operator=(const Attribute& rhs);

    const stringT& name() const { return name_; }
    Type type() const { return type_; }
    DefaultValue default_value() const { return default_value_; }
    const stringT& value() const { return value_; }
    const EnumValueList& enum_values() const { return enum_values_; }

    void set_name(const stringT& name) { name_ = name; }
    void set_type(Type type) { type_ = type; }
    void set_default_value(DefaultValue default_value) { default_value_ = default_value; }
    void set_value(const stringT& value) { value_ = value; }
    void add_enum_value(const stringT& value) { enum_values_.push_back(value); }

    static Type string_to_type(const stringT& str);
    static stringT type_to_string(Type type);
    static DefaultValue string_to_default_value(const stringT& str);
    static stringT default_value_to_string(DefaultValue value);

  private:
    stringT name_;
    Type type_;
    DefaultValue default_value_;
    stringT value_;
    EnumValueList enum_values_;

    typedef std::pair<Type, stringT> type_mapping;
    typedef std::vector<type_mapping> type_mapping_list;
    static const type_mapping_list& type_map();

    typedef std::pair<DefaultValue, stringT> value_mapping;
    typedef std::vector<value_mapping> value_mapping_list;
    static const value_mapping_list& value_map();

    bool operator==(const Attribute&) const; // no_impl
}; // class Attribute

//////////////////////////////////////////////////
// implementation
template<class stringT>
Attribute<stringT>::Attribute(const Attribute<stringT>& rhs) :
  name_(rhs.name_),
  type_(rhs.type_),
  default_value_(rhs.default_value_),
  value_(rhs.value_),
  enum_values_(rhs.enum_values_.begin(), rhs.enum_values_.end())
{
} // Attribute

template<class stringT>
Attribute<stringT>& Attribute<stringT>::operator=(const Attribute<stringT>& rhs)
{
  if(&rhs == this)
    return *this;

  name_ = rhs.name_;
  type_ = rhs.type_;
  default_value_ = rhs.default_value_;
  value_ = rhs.value_;
  enum_values_.clear();
  enum_values_.insert(enum_values_.begin(), rhs.enum_values_.begin(), rhs.enum_values_.end());

  return *this;
} // operator=

////////////////////////////////////////////////////
template<class stringT>
const typename Attribute<stringT>::type_mapping_list& Attribute<stringT>::type_map() 
{
  static type_mapping_list list;

  if(list.empty())
  {
    list.push_back(std::make_pair(CDATA, stringT("CDATA")));
    list.push_back(std::make_pair(NMTOKEN, stringT("NMTOKEN")));
    list.push_back(std::make_pair(NMTOKENS, stringT("NMTOKENS")));
    list.push_back(std::make_pair(ENUMERATION, stringT("ENUMERATION")));
    list.push_back(std::make_pair(ENTITY, stringT("ENTITY")));
    list.push_back(std::make_pair(ENTITIES, stringT("ENTITIES")));
    list.push_back(std::make_pair(ID, stringT("ID")));
    list.push_back(std::make_pair(IDREF, stringT("IDREF")));
    list.push_back(std::make_pair(IDREFS, stringT("IDREFS")));
    list.push_back(std::make_pair(NOTATION, stringT("NOTATION")));
    list.push_back(std::make_pair(UNKNOWN_TYPE, stringT("")));
  } // if ...

  return list;
} // type_map

template<class stringT>
typename Attribute<stringT>::Type Attribute<stringT>::string_to_type(const stringT& str)
{
  const type_mapping_list& map = type_map();
  int i = 0;
  for(; map[i].first != Attribute::UNKNOWN_TYPE; ++i)
    if(str == map[i].second)
      break;
  return map[i].first;
} // string_to_type

template<class stringT>
stringT Attribute<stringT>::type_to_string(typename Attribute<stringT>::Type type)
{
  const type_mapping_list& map = type_map();
  int i = 0;
  for(; map[i].first != Attribute::UNKNOWN_TYPE; ++i)
    if(type == map[i].first)
      break;
  return map[i].second;
} // type_to_string

//////////////////////////////////////////
template<class stringT>
const typename Attribute<stringT>::value_mapping_list& Attribute<stringT>::value_map() 
{
  static value_mapping_list list;

  if(list.empty())
  {
    list.push_back(std::make_pair(IMPLIED, stringT("#IMPLIED")));
    list.push_back(std::make_pair(REQUIRED, stringT("#REQUIRED")));
    list.push_back(std::make_pair(FIXED, stringT("#FIXED")));
    list.push_back(std::make_pair(IMPLIED, stringT("IMPLIED")));
    list.push_back(std::make_pair(REQUIRED, stringT("REQUIRED")));
    list.push_back(std::make_pair(FIXED, stringT("FIXED")));
    list.push_back(std::make_pair(UNKNOWN_VALUE, stringT("")));
  } // if ...

  return list;
} // type_map

template<class stringT>
typename Attribute<stringT>::DefaultValue Attribute<stringT>::string_to_default_value(const stringT& str)
{
  const value_mapping_list& map = value_map();
  int i = 0;
  for(; map[i].first != Attribute::UNKNOWN_VALUE; ++i)
    if(str == map[i].second)
      break;
  return map[i].first;
} // string_to_default_value

template<class stringT>
stringT Attribute<stringT>::default_value_to_string(typename Attribute<stringT>::DefaultValue value)
{
  const value_mapping_list& map = value_map();
  int i = 0;
  for(; map[i].first != Attribute::UNKNOWN_VALUE; ++i)
    if(value == map[i].first)
      break;
  return map[i].second;
} // default_value_to_string

/////////////////////////////////////////////
template<class stringT>
std::ostream& operator<<(std::ostream& os, const Attribute<stringT>& attr)
{
  os << attr.name() 
     << Arabica::Unicode<stringT::value_type>::SPACE
     << Attribute<stringT>::default_value_to_string(attr.default_value());
  if(!attr.value().empty())
    os << Arabica::Unicode<stringT::value_type>::SPACE << Arabica::Unicode<stringT::value_type>::QUOTATION_MARK << attr.value() << Arabica::Unicode<stringT::value_type>::QUOTATION_MARK;
  return os;
} // operator<< 

} // namespace DTD
#endif
