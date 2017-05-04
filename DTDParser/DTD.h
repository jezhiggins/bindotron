#ifndef ARABICA_DTDPARSER_DTD_H
#define ARABICA_DTDPARSER_DTD_H

// $Id: DTD.h,v 1.6 2001/12/13 16:11:55 jez Exp $

#include <ostream>
#include <DTDParser/Element.h>
#include <DTDParser/Notation.h>
#include <list>
#include <vector>
#include <map>
#include <DTDParser/ObjectHasName.h>

namespace DTD
{

template<class stringT>
class DTD
{
  public:
    DTD() { }
    ~DTD() { }

    typedef std::list<Element<stringT> > ElementList;
    typedef std::vector<Notation<stringT> > NotationList;

    const ElementList& elements() const { return elements_; }
    const Element<stringT>& element_by_name(const stringT& name) const;
    ElementList& elements() { return elements_; }

    const NotationList& notations() const { return notations_; }
    NotationList& notations() { return notations_; }

    void validate();

  private:
    ElementList elements_;
    NotationList notations_;

    void checkModelForElement(const ContentModel<stringT>& cm, std::map<stringT, bool>& used_map);

    bool operator==(const DTD&) const; // no impl
    DTD(const DTD& rhs);
    DTD& operator=(const DTD& rhs);
}; // class DTD

template<class stringT>
const Element<stringT>& DTD<stringT>::element_by_name(const stringT& name) const
{
  ElementList::const_iterator e = std::find_if(elements_.begin(), elements_.end(), std::bind2nd(object_has_name<Element<stringT>, stringT>(), name));
  if(e == elements_.end())
    throw std::runtime_error("Named element not found");
  return *e;
} // element_by_name

template<class stringT>
void DTD<stringT>::validate()
{
  ElementList::iterator i;
  for(i = elements_.begin(); i != elements_.end(); ++i)
    if(i->content_model() == ContentModel<stringT>::UNDEFINED)
      throw std::runtime_error("undefined element");

  std::map<stringT, bool> used;
  for(i = elements_.begin(); i != elements_.end(); ++i)
    used[i->name()] = false;
  for(i = elements_.begin(); i != elements_.end(); ++i)
    checkModelForElement(i->content_model(), used);

  std::map<stringT, bool>::const_iterator u;
  for(u = used.begin(); u != used.end(); ++u)
    if((*u).second == false)
      break;

  ElementList::iterator root = std::find_if(elements_.begin(), elements_.end(), std::bind2nd(object_has_name<Element<stringT>, stringT>(), (*u).first));
  if(root != elements_.end())
  {
    Element<stringT> b = *(elements_.begin());
    Element<stringT> r = *root;
    *root = b;
    *(elements_.begin()) = r;
  } // ...
} // validate

template<class stringT>
void DTD<stringT>::checkModelForElement(const ContentModel<stringT>& cm, std::map<stringT, bool>& used_map)
{
  if(cm.type() == ContentModel<stringT>::ELEMENT)
    used_map[cm.element_name()] = true;
  for(int i = 0; i < cm.items().size(); ++i)
    checkModelForElement(cm.items()[i], used_map);
} // checkModelForElement

template<class stringT>
std::ostream& operator<<(std::ostream& os, const DTD<stringT>& dtd)
{
  const DTD<stringT>::NotationList& notations = dtd.notations();
  for(DTD<stringT>::NotationList::const_iterator n = notations.begin(); n != notations.end(); ++n)
    os << *n;

  const DTD<stringT>::ElementList& elements = dtd.elements();
  for(DTD<stringT>::ElementList::const_iterator e = elements.begin(); e != elements.end(); ++e)
    os << *e;

  return os;
} // operator<<


} // namespace DTD

#endif