
#pragma warning(disable: 4786)
#include "DTDModel.h"
#include "Element.h"
#include <DTDParser/Parser.h>
#include <exception>

using namespace BindOTron;

DTDModel::DTDModel()
{
} // DTDModel

DTDModel::~DTDModel()
{
} // ~DTDModel

void DTDModel::loadDTD(SAX::InputSource& inputSource)
{
  DTD::Parser<std::string> parser;
  parser.parse(inputSource);

  rootElement_.set_name(parser.DTD().elements().begin()->name());
  populate(rootElement_, parser.DTD()); 
} // loadDTD

void DTDModel::populate(Element& elem, const DTD::DTD<std::string>& dtd)
{
  DTD::Element<std::string> dtd_elem = dtd.element_by_name(elem.name());
  
  for(DTD::Element<std::string>::AttributeList::const_iterator a = dtd_elem.attributes().begin(); a != dtd_elem.attributes().end(); ++a)
  {
    Attribute attr;
    attr.set_name(a->name());
    attr.set_type(Value::STRING);
    elem.add_attribute(attr);
  } // for ...

  populate_content_model(elem, dtd_elem.content_model(), dtd);
} // populate

void DTDModel::populate_content_model(Element& elem, const DTD::ContentModel<std::string>& content_model, const DTD::DTD<std::string>& dtd)
{
  switch(content_model.type())
  {
    case DTD::ContentModel<std::string>::ANY:
      throw std::runtime_error("Don't know how to handle ANY");
    case DTD::ContentModel<std::string>::EMPTY:
      break;
    case DTD::ContentModel<std::string>::PCDATA:
      elem.set_value();
      break;
    case DTD::ContentModel<std::string>::ELEMENT:
      {
        Element child_elem;
        child_elem.set_name(content_model.element_name());
        populate(child_elem, dtd); 
        if((content_model.cardinality() == DTD::ContentModel<std::string>::OPTIONAL) || 
           (content_model.cardinality() == DTD::ContentModel<std::string>::NONE))
          child_elem.set_type(Value::OBJECT);
        else
          child_elem.set_type(Value::LIST);
        elem.add_element(child_elem);
      } 
      break;
    case DTD::ContentModel<std::string>::CHOICE:
    case DTD::ContentModel<std::string>::SEQUENCE:
      {
        for(DTD::ContentModel<std::string>::ContentModelList::const_iterator cm = content_model.items().begin(); cm != content_model.items().end(); ++cm)
          populate_content_model(elem, *cm, dtd);
      } 
      break;
  } // switch ...
} // populate_content_model

const Element& DTDModel::do_rootElement() const
{
  return rootElement_;
} // rootElementName

// end of file