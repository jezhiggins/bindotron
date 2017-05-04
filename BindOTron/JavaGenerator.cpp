
#pragma warning(disable: 4786)
#include "JavaGenerator.h"
#include "Element.h"
#include <iostream>
#include <functional>
#include <algorithm>

namespace 
{
  void print_type(const BindOTron::Value& val, bool return_type)
  {
    switch(val.type())
    {
      case BindOTron::Value::STRING:
        std::cout << "String";
        break;
      case BindOTron::Value::LIST:
        if(return_type)
          std::cout << "List";
        else
          std::cout << "Vector";
        break;
      case BindOTron::Value::INT:
        std::cout << "int";
        break;
      case BindOTron::Value::FLOAT:
        std::cout << "double";
        break;
    } // switch(val.type())
  } // print_type

  class print_accessor : public std::unary_function<BindOTron::Value, void>
  {
  public:
    print_accessor() { }

    void operator()(const BindOTron::Value& val) const
    {
      std::cout << "  public ";
      print_type(val, true);  
      std::cout << " get_" << val.name() << "() { return " << val.name() << "_; } " << std::endl;
    } // operator()
  }; // print_accessor

  class print_declaration : public std::unary_function<BindOTron::Value, void>
  {
  public:
    print_declaration() { }

    void operator()(const BindOTron::Value& val) const
    {
      std::cout << "  private ";
      print_type(val, false);  
      std::cout << " " << val.name() << "_;" << std::endl;
    } // operator()
  }; // print_accessor

  class print_class_definition : public std::unary_function<BindOTron::Element, void>
  {
  public:
    print_class_definition() { }

    void operator()(const BindOTron::Element& element)
    {
      const std::vector<BindOTron::Attribute>& attrs = element.attributes();
      const std::vector<BindOTron::Element>& elems = element.elements();

      std::cout << "Generating " << element.name() << std::endl;

      std::cout << "public class " << element.name() << std::endl;
      std::cout << "{" << std::endl;

      std::cout << "  // accessors" << std::endl;
      print_accessor pa;
      std::for_each(attrs.begin(), attrs.end(), pa);
      std::for_each(elems.begin(), elems.end(), pa);
      if(element.has_value())
        pa(element.value());
      std::cout << std::endl;

      std::cout << "  // instance variables" << std::endl;
      print_declaration pd;
      std::for_each(attrs.begin(), attrs.end(), pd);
      std::for_each(elems.begin(), elems.end(), pd);
      if(element.has_value())
        pd(element.value());
      std::cout << "} // class " << element.name() << std::endl;
      std::cout << std::endl;

      print_class_definition pcd;
      std::for_each(elems.begin(), elems.end(), pcd);
    } // operator()
  }; // class print_class_definition
} // namespace

namespace BindOTron
{
  void JavaGenerator::do_generate() const
  {
    print_class_definition pcd;
    pcd(model().rootElement());
  } // generate

} // namespace BindOTron

// end of file