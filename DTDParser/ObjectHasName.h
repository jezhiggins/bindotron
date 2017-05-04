#ifndef ARABICA_DTDPARSER_HASNAME_H
#define ARABICA_DTDPARSER_HASNAME_H

// $Id: ObjectHasName.h,v 1.1 2001/12/13 16:12:03 jez Exp $

namespace DTD
{

template<class objectT, class stringT>
class object_has_name : public std::binary_function<objectT, stringT, bool>
{
  public:
    bool operator()(const objectT& o, const stringT& name) const
    {
      return o.name() == name;
    } // operator()
}; // class object_has_name

} // namespace DTD

#endif