
#pragma warning(disable: 4786)
#include "TestModel.h"
#include "Element.h"
#include <exception>

namespace BindOTron
{
  TestModel::TestModel()
  {
    Attribute a;
    a.set_name("spacing");

    Element e2;
    e2.set_name("para");
    e2.set_value();
    e2.add_attribute(a);

    Element e3;
    e3.set_name("header");
    e3.set_type(Value::OBJECT);

    rootElement.set_name("doc");
    rootElement.add_element(e3);
    rootElement.add_element(e2);
  } // TestModel

  TestModel::~TestModel()
  {
  } // ~TestModel

  const Element& TestModel::do_rootElement() const
  {
    return rootElement;
  } // rootElementName
} // namespace BindOTron

// end of file