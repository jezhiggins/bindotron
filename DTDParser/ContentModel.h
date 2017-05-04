#ifndef ARABICA_DTDPARSER_CONTENTMODEL_H
#define ARABICA_DTDPARSER_CONTENTMODEL_H

// $Id: ContentModel.h,v 1.4 2001/12/06 09:38:53 jez Exp $

#include <vector>
#include <ostream>

namespace DTD
{

template<class stringT>
class ContentModel
{
  public:
    enum Type 
    {
      ANY,
      EMPTY,
      PCDATA,
      ELEMENT,
      CHOICE,
      SEQUENCE,
      UNDEFINED
    };

    enum Cardinality
    {
      OPTIONAL,
      ZEROORMORE,
      ONEORMORE,
      NONE,
      UNKNOWN_CARDINALITY
    };

    ContentModel() : type_(UNDEFINED), card_(NONE) { }
    ContentModel(Type type) : type_(type), card_(NONE) { }
    explicit ContentModel(const stringT& element_name) : type_(ELEMENT), card_(NONE), element_name_(element_name) { }
    ContentModel(const ContentModel& rhs);
    ~ContentModel() { }
    ContentModel& operator=(const ContentModel& rhs);
    bool operator==(Type type) const;
    bool operator!=(Type type) const;

    typedef std::vector<ContentModel> ContentModelList;

    Type type() const { return type_; }
    Cardinality cardinality() const { return card_; }
    const ContentModelList& items() const { return items_; }
    const stringT& element_name() const { return element_name_; }

    void set_type(Type type) { type_ = type; }
    void set_cardinality(Cardinality card) { card_ = card; }
    void add_item(const ContentModel& content_model) { items_.push_back(content_model); }

    static Cardinality string_to_cardinality(const stringT& str);
    static stringT cardinality_to_string(Cardinality cardinality);

  private:
    Type type_;
    Cardinality card_;
    ContentModelList items_;
    stringT element_name_;

    typedef std::pair<Cardinality, stringT> cardinality_mapping;
    typedef std::vector<cardinality_mapping> cardinality_mapping_list;
    static const cardinality_mapping_list& cardinality_map();

    bool operator==(const ContentModel&) const; // impl
}; // class ContentModel

///////////////////////////////
// implementation
template<class stringT>
ContentModel<stringT>::ContentModel(const ContentModel<stringT>& rhs) :
  type_(rhs.type_),
  card_(rhs.card_),
  items_(rhs.items_.begin(), rhs.items_.end()),
  element_name_(rhs.element_name_)
{
} // ContentModel

template<class stringT>
ContentModel<stringT>& ContentModel<stringT>::operator=(const ContentModel<stringT>& rhs)
{
  if(&rhs == this)
    return *this;

  type_ = rhs.type_;
  card_ = rhs.card_;
  element_name_ = rhs.element_name_;
  items_.clear();
  items_.insert(items_.begin(), rhs.items_.begin(), rhs.items_.end());

  return *this;
} // operator=

template<class stringT>
bool ContentModel<stringT>::operator==(Type type) const
{
  return type_ == type;
} // operator==

template<class stringT>
bool ContentModel<stringT>::operator!=(Type type) const
{
  return !(operator==(type));
} // operator!=

template<class stringT>
std::ostream& operator<<(std::ostream& os, const ContentModel<stringT>& cm)
{
  typedef Arabica::Unicode<stringT::value_type> UnicodeT;

  switch(cm.type())
  {
    case ContentModel<stringT>::ANY:
      os << "ANY";
      break;
    case ContentModel<stringT>::EMPTY:
      os << "EMPTY";
      break;
    case ContentModel<stringT>::PCDATA:
      os << "#PCDATA";
      break;
    case ContentModel<stringT>::ELEMENT:
      os << cm.element_name().c_str();
      os << ContentModel<stringT>::cardinality_to_string(cm.cardinality()).c_str();
      break;
    case ContentModel<stringT>::CHOICE:
    case ContentModel<stringT>::SEQUENCE:
      {
        stringT::value_type sep = ((cm.type() == ContentModel<stringT>::CHOICE) ? UnicodeT::VERTICAL_BAR : UnicodeT::COMMA);
        os << UnicodeT::LEFT_PARENTHESIS;
        for(int i = 0; i < cm.items().size(); ++i)
        {
          if(i > 0)
            os << sep;
          os << cm.items()[i];
        }     
        os << UnicodeT::RIGHT_PARENTHESIS;

        os << ContentModel<stringT>::cardinality_to_string(cm.cardinality());
      }
      break;
  } // switch

  return os;
} // operator<<

//////////////////////////////////////////
template<class stringT>
const typename ContentModel<stringT>::cardinality_mapping_list& ContentModel<stringT>::cardinality_map() 
{
  static cardinality_mapping_list list;

  if(list.empty())
  {
    list.push_back(std::make_pair(OPTIONAL, stringT("?")));
    list.push_back(std::make_pair(ZEROORMORE, stringT("*")));
    list.push_back(std::make_pair(ONEORMORE, stringT("+")));
    list.push_back(std::make_pair(UNKNOWN_CARDINALITY, stringT("")));
  } // if ...

  return list;
} // type_map

template<class stringT>
typename ContentModel<stringT>::Cardinality ContentModel<stringT>::string_to_cardinality(const stringT& str)
{
  const cardinality_mapping_list& map = cardinality_map();
  int i = 0;
  for(; map[i].first != ContentModel<stringT>::UNKNOWN_CARDINALITY; ++i)
    if(map[i].second == str)
      break;
  return map[i].first;
} // string_to_cardinality

template<class stringT>
stringT ContentModel<stringT>::cardinality_to_string(typename ContentModel<stringT>::Cardinality cardinality)
{
  const cardinality_mapping_list& map = cardinality_map();
  int i = 0;
  for(; map[i].first != ContentModel<stringT>::UNKNOWN_CARDINALITY; ++i)
    if(cardinality == map[i].first)
      break;
  return map[i].second;
} // cardinality_to_string

} // namespace DTD

#endif