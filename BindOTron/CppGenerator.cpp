
#pragma warning(disable: 4786)
#include "CppGenerator.h"
#include "Element.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <algorithm>

namespace 
{
  std::string class_header_filename(const BindOTron::Element& element)
  {
    std::string filename = element.name() + ".h";
    return filename;
  } // class_header_filename

  std::string class_implementation_filename(const BindOTron::Element& element)
  {
    std::string filename = element.name() + ".cpp";
    return filename;
  } // class_implementation_filename

  enum WhereType { RETURN, PARAMETER, DECL };
  void print_type(std::ostream& os, const BindOTron::Value& val, WhereType where)
  {
    switch(val.type())
    {
      case BindOTron::Value::STRING:
        if(where == PARAMETER) os << "const ";
        os << "std::string";
        if(where == PARAMETER) os << "&";
        break;
      case BindOTron::Value::LIST:
        if(where == RETURN) os << "const ";
        os << "std::vector<" << val.name() << ">";
        if(where == RETURN) os << "&";
        break;
      case BindOTron::Value::INT:
        os << "int";
        break;
      case BindOTron::Value::FLOAT:
        os << "double";
        break;
      case BindOTron::Value::OBJECT:
        if(where == RETURN || where == PARAMETER) os << "const ";
        os << val.name();
        if(where == RETURN || where == PARAMETER) os << "&";
    } // switch(val.type())
  } // print_type

  void print_type_default(std::ostream& os, const BindOTron::Value& val)
  {
    switch(val.type())
    {
      case BindOTron::Value::INT:
      case BindOTron::Value::FLOAT:
        os << "0";
        break;
      default:
        break;
    } // switch(val.type())
  } // print_type_default

  template<typename Fn>
  void print_something(const BindOTron::Element& element, Fn thing) 
  {
    const std::vector<BindOTron::Attribute>& attrs = element.attributes();
    const std::vector<BindOTron::Element>& elems = element.elements();

    std::for_each(attrs.begin(), attrs.end(), thing);
    std::for_each(elems.begin(), elems.end(), thing);
    if(element.has_value())
      thing(element.value());
  } // print_something

  class print_getter_declaration : public std::unary_function<BindOTron::Value, void>
  {
    public:
      print_getter_declaration(std::ostream& os) : os_(os) { }

      void operator()(const BindOTron::Value& val) const
      {
        os_ << "    ";
        print_type(os_, val, RETURN);  
        os_ << " get_" << val.name() << "() const;" << std::endl;
      } // operator()

    private:
      std::ostream& os_;
  }; // print_getter_declaration

  class print_getter_handle_definition : public std::binary_function<BindOTron::Value, BindOTron::Element, bool>
  {
    public:
      print_getter_handle_definition(std::ostream& os) : os_(os) { }

      bool operator()(const BindOTron::Value& val, const BindOTron::Element& element) const
      {
        print_type(os_, val, RETURN);  
        os_ << " " << element.name() << "::get_" << val.name() << "() const" << std::endl
            << "{" << std::endl
            << "  return impl_->" << val.name() << "_;" << std::endl
            << "} // get_" << val.name() << std::endl << std::endl;
        return true;
      } // operator()

    private:
      std::ostream& os_;
  }; // print_getter_handle_definition

  class print_setter_declaration : public std::unary_function<BindOTron::Value, void>
  {
    public:
      print_setter_declaration(std::ostream& os) : os_(os) { }

      void operator()(const BindOTron::Value& val) const
      {
        if(val.type() != BindOTron::Value::LIST)
          print_simple(val);
        else
          print_list(val);
      } // operator()

      void print_simple(const BindOTron::Value& val) const
      {
        os_ << "    void set_" << val.name() << "(";
        print_type(os_, val, PARAMETER);
        os_ << " " << val.name() << ");" << std::endl;
      } // operator()

      void print_list(const BindOTron::Value& val) const
      {
        os_ << "    void add_" << val.name() << "(const " << val.name() << "& new_" << val.name() << ");" << std::endl;
        os_ << "    void remove_" << val.name() << "(const " << val.name() << "& old_"<< val.name() << ");" << std::endl;
      } // print_list

    private:
      std::ostream& os_;
  }; // print_setter_declaration

  class print_setter_handle_definition : public std::binary_function<BindOTron::Value, BindOTron::Element, bool>
  {
    public:
      print_setter_handle_definition(std::ostream& os) : os_(os) { }

      bool operator()(const BindOTron::Value& val, const BindOTron::Element& element) const
      {
        if(val.type() != BindOTron::Value::LIST)
          print_simple(val, element);
        else
          print_list(val, element);
        return true;
      } // operator()

      void print_simple(const BindOTron::Value& val, const BindOTron::Element& element) const
      {
        os_ << "void " << element.name() << "::set_" << val.name() << "(";
        print_type(os_, val, PARAMETER);
        os_ << " " << val.name() << ")" << std::endl
            << "{" << std::endl
            << "  impl_->" << val.name() << "_ = " << val.name() << ";" << std::endl 
            << "} // set_" << val.name() << std::endl << std::endl;
      } // operator()

      void print_list(const BindOTron::Value& val, const BindOTron::Element& element) const
      {
        os_ << "void " << element.name() << "::add_" << val.name() << "(const " << val.name() << "& new_" << val.name() << ")" << std::endl 
            << "{ " << std::endl 
            << "  impl_->" << val.name() << "_.push_back(new_" << val.name() << ");" << std::endl 
            << "} // add_" << val.name() << std::endl << std::endl;

        os_ << "void " << element.name() << "::remove_" << val.name() << "(const " << val.name() << "& old_" << val.name() << ")" << std::endl 
                  << "{ " << std::endl
                  << "  ";
        print_type(os_, val, DECL);
        os_ << "::iterator " << val.name() << "_iterator = std::find(impl_->" << val.name() << "_.begin(), impl_->" << val.name() << "_.end(), old_" << val.name() << ");" << std::endl;
        os_ << "  if(" << val.name() << "_iterator != impl_->" << val.name() << "_.end())" << std::endl
            << "    impl_->" << val.name() << "_.erase(" << val.name() << "_iterator);" << std::endl
            << "} // remove_" << val.name() << std::endl << std::endl;
      } // print_list

    private:
      std::ostream& os_;
  }; // print_setter_handle_definition

  class print_variable_declaration : public std::unary_function<BindOTron::Value, void>
  {
    public:
      print_variable_declaration(std::ostream& os) : os_(os) { }

      void operator()(const BindOTron::Value& val) const
      {
        os_ << "    ";
        print_type(os_, val, DECL);  
        os_ << " " << val.name() << "_;" << std::endl;
      } // operator()

    private:
      std::ostream& os_;
  }; // print_variable_declaration 

  class print_variable_initialiser : public std::unary_function<BindOTron::Value, void>
  {
    public:
      print_variable_initialiser(std::ostream& os) : os_(os) { }

      void operator()(const BindOTron::Value& val) const
      {
        os_ << "," << std::endl;
        os_ << "      " << val.name() << "_(";
        print_type_default(os_, val);
        os_ << ")";
      } // operator()

    private:
      std::ostream& os_;
  }; // print_variable_initialiser

  class print_create_child_factory : public std::unary_function<BindOTron::Element, void>
  {
    public:
      print_create_child_factory(std::ostream& os) : os_(os) { }

      void operator()(const BindOTron::Element& element) const
      {
        os_ << "  if(localName == \"" << element.name() << "\")" << std::endl
            << "    child_factory_ = new " << element.name() << "_factory(parser_, *this);" << std::endl;
      } // operator()

    private:
      std::ostream& os_;
  }; // print_create_child_factory

  class print_get_child_factory_value : public std::unary_function<BindOTron::Element, void>
  {
    public:
      print_get_child_factory_value(std::ostream& os, const BindOTron::Element& parent) : os_(os), parent_(parent) { }

      void operator()(const BindOTron::Element& element) const
      {
        os_ << "  if(localName == \"" << element.name() << "\")" << std::endl
            << "    " << parent_.name() << "_.";
        if(element.type() == BindOTron::Value::OBJECT)
          os_ << "set";
        else
          os_ << "add";
        os_ << "_" << element.name() << "(dynamic_cast<" << element.name() << "_factory*>(child_factory_)->get_" << element.name() << "());" << std::endl;
      } // operator()

    private:
      std::ostream& os_;
      const BindOTron::Element& parent_;
  }; // print_get_child_factory_value

  class print_set_attribute_value : public std::unary_function<BindOTron::Attribute, void>
  {
    public:
      print_set_attribute_value(std::ostream& os, const BindOTron::Element& element) : os_(os), element_(element) { }

      void operator()(const BindOTron::Attribute& attr) const
      {
        os_ << "      if(atts.getLocalName(i) == \"" << attr.name() << "\")" << std::endl
            << "        " << element_.name() << "_.set_" << attr.name() << "(atts.getValue(i));" << std::endl;
      } // operator()

    private:
      std::ostream& os_;
      const BindOTron::Element& element_;
  }; // print_set_attribute_value

  class print_handle_class_declaration : public std::unary_function<BindOTron::Element, void>
  {
    public:
      print_handle_class_declaration() : root_(false) { }
      print_handle_class_declaration(bool root) : root_(root) { }

      void operator()(const BindOTron::Element& element);

    private:
      void print_header(std::ostream& os, const BindOTron::Element& element) const;
      void print_big_five_declarations(std::ostream& os, const BindOTron::Element& element) const;
      void print_getter_declarations(std::ostream& os, const BindOTron::Element& element) const; 
      void print_setter_declarations(std::ostream& os, const BindOTron::Element& element) const; 
      void print_xml_emitter_declaration(std::ostream& os, const BindOTron::Element& element) const; 
      void print_footer(std::ostream& os, const BindOTron::Element& element) const; 
      void print_factory_declaration(std::ostream& os, const BindOTron::Element& element) const; 

      bool root_;
  }; // class print_handle_class_declaration

  class print_handle_class_definition : public std::unary_function<BindOTron::Element, void>
  {
    public:
      print_handle_class_definition() : root_(false) { }
      print_handle_class_definition(bool root) : root_(root) { }

      void operator()(const BindOTron::Element& element);

    private:
      void print_header(std::ostream& os, const BindOTron::Element& element) const;
      void print_big_five_definitions(std::ostream& os, const BindOTron::Element& element) const;
      void print_getter_definitions(std::ostream& os, const BindOTron::Element& element) const;
      void print_setter_definitions(std::ostream& os, const BindOTron::Element& element) const;
      void print_xml_emitter_definition(std::ostream& os, const BindOTron::Element& element) const;
      void print_footer(std::ostream& os, const BindOTron::Element& element) const;
      void print_factory_definition(std::ostream& os, const BindOTron::Element& element) const; 

      bool root_;
  }; // class print_handle_class_definition

  class print_body_class_definition : public std::unary_function<BindOTron::Element, void>
  {
    public:
      print_body_class_definition(std::ostream& os) : os_(os) { }

      void operator()(const BindOTron::Element& element) const;

    private:
      void print_header(const BindOTron::Element& element) const;
      void print_member_variables(const BindOTron::Element& element) const;
      void print_footer(const BindOTron::Element& element) const;

    private:
      std::ostream& os_;
  }; // class print_body_class_definition
} // namespace

namespace BindOTron
{
  void CppGenerator::do_generate() const
  {
    print_handle_class_declaration phcd(true);
    phcd(model().rootElement());

    print_handle_class_definition phcdef(true);
    phcdef(model().rootElement());
  } // generate
} // namespace BindOTron

/////////////////////////////////////////////////////////////////////
// print_handle_class_declaration
void print_handle_class_declaration::operator()(const BindOTron::Element& element) 
{
  std::ofstream of(class_header_filename(element).c_str());

  of << "#ifndef BINDOTRON_HEADER_GUARD_" << element.name() << std::endl
     << "#define BINDOTRON_HEADER_GUARD_" << element.name() << std::endl
     << std::endl;

  print_header(of, element);
  print_big_five_declarations(of, element);
  print_getter_declarations(of, element);
  print_setter_declarations(of, element);
  print_xml_emitter_declaration(of, element);
  print_footer(of, element);

  print_factory_declaration(of, element);

  of << "#endif" << std::endl
     << std::endl;

  of.close();
  
  std::cout << "Generated " << class_header_filename(element) << std::endl;
  
  const std::vector<BindOTron::Element>& elems = element.elements();
  print_handle_class_declaration phcd;
  std::for_each(elems.begin(), elems.end(), phcd);
} // operator()
    
void print_handle_class_declaration::print_header(std::ostream& os, const BindOTron::Element& element) const
{
  os << "#include <string>" << std::endl
     << "#include <vector>" << std::endl
     << "#include <SAX/XMLReader.h>" << std::endl
     << "#include <SAX/helpers/DefaultHandler.h>" << std::endl;

  const std::vector<BindOTron::Element>& elems = element.elements();
  for(std::vector<BindOTron::Element>::const_iterator e = elems.begin(); e != elems.end(); ++e)
    os << "#include \"" << e->name() << ".h\"" << std::endl;
  os << std::endl;

  os << "class " << element.name() << "_impl;" << std::endl << std::endl;

  os << "class " << element.name() << std::endl
     << "{" << std::endl
     << "  public: " << std::endl;
} // print_header

void print_handle_class_declaration::print_big_five_declarations(std::ostream& os, const BindOTron::Element& element) const
{
  os << "    " << element.name() << "();" << std::endl 
     << "    " << element.name() << "(const " << element.name() << "& rhs);" << std::endl
     << "    ~" << element.name() << "();" << std::endl << std::endl
     << "    " << element.name() << "& operator=(const " << element.name() << "& rhs);" << std::endl
     << "    bool operator==(const " << element.name() << "& rhs) const;" << std::endl << std::endl;
} // print_big_five_declarations

void print_handle_class_declaration::print_getter_declarations(std::ostream& os, const BindOTron::Element& element) const 
{
  os << "    ////////////////" << std::endl;
  os << "    // getters" << std::endl;
  print_something(element, print_getter_declaration(os));
  os << std::endl;
} // print_getter_declarations

void print_handle_class_declaration::print_setter_declarations(std::ostream& os, const BindOTron::Element& element) const 
{
  os << "    ////////////////" << std::endl;
  os << "    // setters" << std::endl;
  print_something(element, print_setter_declaration(os));
  os << std::endl;
} // print_setter_declarations

void print_handle_class_declaration::print_xml_emitter_declaration(std::ostream& os, const BindOTron::Element& element) const
{
  os << "    ////////////////" << std::endl
     << "    // emit_xml" << std::endl
     << "    void emit_xml(std::ostream& os) const;" << std::endl << std::endl;
} // print_xml_emitter_declaration

void print_handle_class_declaration::print_footer(std::ostream& os, const BindOTron::Element& element) const
{
  os << "  private: " << std::endl
     << "    void set_impl(" << element.name() << "_impl* impl);" << std::endl
     << "    " << element.name() << "_impl* impl_;" << std::endl
     << "}; // class " << element.name() << std::endl
     << std::endl;
} // print_footer

void print_handle_class_declaration::print_factory_declaration(std::ostream& os, const BindOTron::Element& element) const 
{
  os << "class " << element.name() << "_factory : public SAX::DefaultHandler" << std::endl
     << "{" << std::endl
     << "  public:" << std::endl;
  if(root_)
    os << "    static " << element.name() << " read(SAX::InputSource& is);" << std::endl
       << std::endl
       << "  private:" << std::endl;
  
  os << "    " << element.name() << "_factory(SAX::XMLReader& reader";
  if(!root_)
    os << ", SAX::ContentHandler& parent_factory";
  os << ");" << std::endl
     << "    ~" << element.name() << "_factory();" << std::endl
     << std::endl
     << "    const " << element.name() << "& get_" << element.name() << "() const;" << std::endl
     << std::endl
     << "  private:" << std::endl
     << "    virtual void startElement(const std::string& namespaceURI, const std::string& localName, const std::string& qName, const SAX::Attributes& atts);" << std::endl
     << "    virtual void endElement(const std::string& namespaceURI, const std::string& localName, const std::string& qName);" << std::endl
     << "    virtual void characters(const std::string& ch);" << std::endl
     << std::endl;

  if(root_)
    os << "    virtual void fatalError(const SAX::SAXException& exception);" << std::endl << std::endl;

  os << "    " << element.name() << " " << element.name() << "_;" << std::endl
     << "    SAX::XMLReader& parser_;" << std::endl;

  if(!root_)
    os << "    SAX::ContentHandler& parent_factory_;" << std::endl;
  if(element.elements().size())
    os << "    SAX::ContentHandler* child_factory_;" << std::endl;

  os << "}; // class " << element.name() << "_factory" << std::endl;
} // print_factory_declaration

////////////////////////////////////////////////////////////////////////
// print_handle_class_definition 
void print_handle_class_definition::operator()(const BindOTron::Element& element)
{
  std::ofstream of(class_implementation_filename(element).c_str());

  print_header(of, element);

  print_big_five_definitions(of, element);

  print_getter_definitions(of, element);
  print_setter_definitions(of, element);

  print_xml_emitter_definition(of, element);

  print_footer(of, element);

  print_factory_definition(of, element);
  
  of << "// end of file " << std::endl;

  of.close();

  std::cout << "Generated " << class_implementation_filename(element) << std::endl;

  const std::vector<BindOTron::Element>& elems = element.elements();
  print_handle_class_definition phcd;
  std::for_each(elems.begin(), elems.end(), phcd);
} // operator()

void print_handle_class_definition::print_header(std::ostream& os, const BindOTron::Element& element) const
{
  if(root_)
    os << "#include <SAX/wrappers/saxmsxml2.h>" << std::endl;
  os << "#include <algorithm>" << std::endl
     << "#include \"" << element.name() << ".h\"" << std::endl
     << std::endl;

  print_body_class_definition pbcd(os);
  pbcd(element);

  os << std::endl
     << "/////////////////////////////////////////" << std::endl
     << "// " << element.name() << " definition" << std::endl;
} // print_header

void print_handle_class_definition::print_big_five_definitions(std::ostream& os, const BindOTron::Element& element) const
{
  // default_constructor
  os << element.name() << "::" << element.name() << "() :" << std::endl
     << "  impl_(0)" << std::endl
     << "{" << std::endl 
     << "  set_impl(new " << element.name() << "_impl());" << std::endl 
     << "} // " << element.name() << std::endl << std::endl;

  // copy constructor
  os << element.name() << "::" << element.name() << "(const " << element.name() << "& rhs) :" << std::endl
     << "  impl_(0)" << std::endl
     << "{" << std::endl 
     << "  set_impl(rhs.impl_);" << std::endl 
     << "} // " << element.name() << std::endl << std::endl;

  // destructor
  os << element.name() << "::~" << element.name() << "()" << std::endl
     << "{" << std::endl 
     << "  set_impl(0);" << std::endl
     << "} // ~" << element.name() << std::endl << std::endl;

  // operator=
  os << element.name() << "& " << element.name() << "::operator=(const " << element.name() << "& rhs)" << std::endl
     << "{" << std::endl
     << "  set_impl(rhs.impl_);" << std::endl
     << "  return *this;" << std::endl
     << "} // operator=" << std::endl << std::endl;

  // operator==
  os << "bool " << element.name() << "::operator==(const " << element.name() << "& rhs) const" << std::endl
     << "{" << std::endl
     << "  return impl_ == rhs.impl_;" << std::endl
     << "} // operator==" << std::endl << std::endl;
} // print_big_five_definitions

void print_handle_class_definition::print_getter_definitions(std::ostream& os, const BindOTron::Element& element) const
{
  os << "////////////////" << std::endl;
  os << "// getters" << std::endl;
  print_something(element, std::bind2nd(print_getter_handle_definition(os), element));
  os << std::endl;
} // print_getter_definitions

void print_handle_class_definition::print_setter_definitions(std::ostream& os, const BindOTron::Element& element) const
{
  os << "////////////////" << std::endl;
  os << "// setters" << std::endl;
  print_something(element, std::bind2nd(print_setter_handle_definition(os), element));
  os << std::endl;
} // print_setter_definitions

void print_handle_class_definition::print_xml_emitter_definition(std::ostream& os, const BindOTron::Element& element) const
{
  os << "////////////////" << std::endl;
  os << "// emit_xml" << std::endl;
  os << "void " << element.name() << "::emit_xml(std::ostream& os) const" << std::endl
     << "{" << std::endl;

  os << "  os << \"<" << element.name() << "\"" << std::endl;
  const std::vector<BindOTron::Attribute>& attrs = element.attributes();
  for(std::vector<BindOTron::Attribute>::const_iterator a = attrs.begin(); a != attrs.end(); ++a)
    os << "     << \" " << a->name() << "=\\\"\" << impl_->" << a->name() << "_ << \"\\\"\" "<< std::endl;
  os << "     << \'>\'; " << std::endl;

  const std::vector<BindOTron::Element>& elems = element.elements();
  for(std::vector<BindOTron::Element>::const_iterator e = elems.begin(); e != elems.end(); ++e)
  {
    if(e->type() == BindOTron::Value::LIST)
    {
      std::string it_name = e->name() + "_iterator";
      os << "  for(std::vector<" << e->name() << ">::const_iterator " << it_name << " = impl_->" << e->name() << "_.begin();" << std::endl
         << "      " << it_name << " != impl_->" << e->name() << "_.end(); ++" << it_name << ")" << std::endl;
      os << "    " << it_name << "->emit_xml(os);" << std::endl;
    }
    else
    {
      os << "    impl_->" << e->name() << "_.emit_xml(os);" << std::endl;
    } // if ... 
  } // for 

  if(element.has_value())
    os << "  os << impl_->value_; " << std::endl;

  os << "  os << \"</" << element.name() << ">\" << std::endl; " << std::endl 
     << "} // emit_xml" << std::endl << std::endl;
} // print_xml_emitter_definition

void print_handle_class_definition::print_footer(std::ostream& os, const BindOTron::Element& element) const 
{
  os << "void " << element.name() << "::set_impl(" << element.name() << "_impl* impl)" << std::endl
     << "{" << std::endl
     << "  if(impl_)" << std::endl
     << "  {" << std::endl
     << "    impl_->remove_ref();" << std::endl
     << "    impl_ = 0;" << std::endl
     << "  }" << std::endl
     << "  if(impl)" << std::endl
     << "  {" << std::endl
     << "    impl_ = impl;" << std::endl
     << "    impl_->add_ref();" << std::endl
     << "  }" << std::endl
     << "} // set_impl" << std::endl
     << std::endl;
} // print_footer

void print_handle_class_definition::print_factory_definition(std::ostream& os, const BindOTron::Element& element) const 
{
  os << "/////////////////////////////////////" << std::endl
     << "// " << element.name() << "_factory : public SAX::DefaultHandler" << std::endl;
  
  if(root_)
    os << element.name() << " " << element.name() << "_factory::read(SAX::InputSource& is)" << std::endl
       << "{" << std::endl
       << "  SAX::msxml2_wrapper<std::string> parser;" << std::endl
       << "  " << element.name() << "_factory f(parser);" << std::endl
       << std::endl
       << "  parser.setContentHandler(f);" << std::endl
       << "  parser.setErrorHandler(f);" << std::endl
       << "  parser.parse(is);" << std::endl
       << std::endl
       << "  return f.get_" << element.name() << "();" << std::endl
       << "} // read" << std::endl
       << std::endl;

  os << element.name() << "_factory::" << element.name() << "_factory(SAX::XMLReader& parser";
  if(!root_)
    os << ",  SAX::ContentHandler& parent_factory";
  os << ") :" << std::endl
     << "    parser_(parser)";
  if(element.elements().size())
    os << std::endl << ",    child_factory_(0)";
  if(!root_)
    os << std::endl << ",    parent_factory_(parent_factory)";
  os << std::endl
     << "{" << std::endl
     << "} // " << element.name() << "_factory" << std::endl
     << std::endl
     << element.name() << "_factory::~" << element.name() << "_factory()" << std::endl
     << "{" << std::endl
     << "} // ~" << element.name() << "_factory" << std::endl
     << std::endl
     << "const " << element.name() << "& " << element.name() << "_factory::get_" << element.name() << "() const" << std::endl
     << "{" << std::endl
     << "  return " << element.name() << "_;" << std::endl
     << "} // get_" << element.name() << std::endl
     << std::endl;

  // startElement
  os << "void " << element.name() << "_factory::startElement(const std::string& namespaceURI, const std::string& localName, const std::string& qName, const SAX::Attributes& atts)" << std::endl
     << "{" << std::endl;

  if(element.attributes().size())
  {
    os << "  if(localName == \"" << element.name() << "\")" << std::endl
       << "  {" << std::endl
       << "    for(int i = 0; i < atts.getLength(); ++i)" << std::endl
       << "    {" << std::endl;
    std::for_each(element.attributes().begin(), element.attributes().end(), print_set_attribute_value(os, element));
    os << "    }" << std::endl
       << "    return;" << std::endl
       << "  }" << std::endl;
  } 
  if(element.elements().size())
  {
    os << "  if(child_factory_) " << std::endl
       << "  {" << std::endl
       << "    delete child_factory_;" << std::endl
       << "    child_factory_ = 0;" << std::endl
       << "  }" << std::endl << std::endl;
    std::for_each(element.elements().begin(), element.elements().end(), print_create_child_factory(os));
    os << std::endl
       << "  if(child_factory_) " << std::endl
       << "  {" << std::endl
       << "    parser_.setContentHandler(*child_factory_);" << std::endl
       << "    child_factory_->startElement(namespaceURI, localName, qName, atts);" << std::endl
       << "  }" << std::endl << std::endl;
  }
  os << "} // startElement" << std::endl
     << std::endl;

  // end element
  os << "void " << element.name() << "_factory::endElement(const std::string& namespaceURI, const std::string& localName, const std::string& qName)" << std::endl
     << "{" << std::endl;
  if(!root_)
    os << "  if(localName == \"" << element.name() << "\")" << std::endl
       << "  {" << std::endl
       << "    parser_.setContentHandler(parent_factory_);" << std::endl
       << "    parent_factory_.endElement(namespaceURI, localName, qName);" << std::endl
       << "  } // if(localName == \"" << element.name() << "\")" << std::endl;
    std::for_each(element.elements().begin(), element.elements().end(), print_get_child_factory_value(os, element));
    os << "} // endElement" << std::endl
     << std::endl;

  os << "void " << element.name() << "_factory::characters(const std::string& ch)" << std::endl
     << "{" << std::endl;
  if(element.has_value())
    os << "  " << element.name() << "_.set_value(" << element.name() << "_.get_value() + ch);" << std::endl;
  os << "} // characters" << std::endl
     << std::endl;

  if(root_)
    os << "void " << element.name() << "_factory::fatalError(const SAX::SAXException& exception)" << std::endl
       << "{" << std::endl
       << "  " << element.name() << "_ = " << element.name() << "();" << std::endl
       << "} // fatalError" << std::endl;
} // print_factory_definition

////////////////////////////////////////////////////////////////////////
// print_body_class_definition 
void print_body_class_definition::operator()(const BindOTron::Element& element) const
{
  print_header(element);
  print_member_variables(element);
  print_footer(element);
} // print_body_class_definition

void print_body_class_definition::print_header(const BindOTron::Element& element) const
{
  os_ << "class " << element.name() << "_impl" << std::endl
      << "{" << std::endl
      << "  public:" << std::endl
      << "    " << element.name() << "_impl() : " << std::endl
      << "      ref_count(0)";

  print_something(element, print_variable_initialiser(os_));

  os_ << std::endl
      << "    {" << std::endl
      << "    } // " << element.name() << std::endl
      << std::endl
      << "    ~" << element.name() << "_impl()" << std::endl
      << "    {" << std::endl
      << "    } // ~" << element.name() << std::endl
      << std::endl;
} // print_header

void print_body_class_definition::print_member_variables(const BindOTron::Element& element) const
{
  os_ << "    ////////////////" << std::endl
      << "    // instance variables" << std::endl;
  print_something(element, print_variable_declaration(os_));
} // print_member_variables

void print_body_class_definition::print_footer(const BindOTron::Element& element) const
{
  os_ << std::endl
      << "    ////////////////" << std::endl
      << "    // ref counting" << std::endl
      << "    void add_ref()" << std::endl
      << "    {" << std::endl 
      << "      ++ref_count;" << std::endl
      << "    } // add_ref" << std::endl
      << "    void remove_ref()" << std::endl
      << "    {" << std::endl 
      << "      if(--ref_count == 0)" << std::endl
      << "        delete this; " << std::endl
      << "    } // remove_ref" << std::endl
      << std::endl
      << "  private:" << std::endl
      << "    unsigned int ref_count;" << std::endl
      << std::endl
      << "    // no impl" << std::endl
      << "    " << element.name() << "_impl(const " << element.name() << "_impl&);" << std::endl
      << "    " << element.name() << "_impl& operator=(const " << element.name() << "_impl&);" << std::endl
      << "    bool operator==(const " << element.name() << "_impl&) const;" << std::endl
      << "}; // class " << element.name() << "_impl" << std::endl;
} // print_footer

// end of file