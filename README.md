## BindOTron

BindOTron is an early experiment in XML databinding, which I'm releasing now for historical interest. BindOTron reads a DTD and generates a C++ or Java class hierarchy mirroring the DTD's structure. Pretty much, anyway. It maps elements to classes and attributes to member variables (properties).

The C++ support is pretty good actually, generating classes, deserialisation stacks of SAX filters and XML output serialisation. The classes it creates are value classes, so includes all the handle-body ref-counting gubbins you need to pass around arbitrarily large object trees cheaply. The Java support is pretty token, generating only the class definitions, and largely exists to prove that once you have an in-memory representation of the DTD creating class hierarchies in whatever language is a case of cranking the handle.

Given that DTDs were never intended to define anything other than document structure, generating class hierarchies from them wasn't ever going to produce sparkling code. It is possible to produce reasonable and useable code though, as BindOTron demonstrates. These days, schema languages like RelaxNG and W3C XML Schema provide a much richer base for code generation, and its use is quite wide spread in many areas. That's particularly true for Java - see JAXB and Zeus for instance. They're quite large and full-featured bits of kit though, and I suspect there's still room for BindOTron style quick'n'dirty/straightforward binding.

### Example Output

Given the following DTD:

```
<!ELEMENT catalog (book+) >

<!ELEMENT book (author, title, genre, price, publish_date, description) >
<!ATTLIST book id ID #REQUIRED >
<!ELEMENT author (#PCDATA) >
<!ELEMENT title (#PCDATA) >
<!ELEMENT genre (#PCDATA) >
<!ELEMENT price (#PCDATA) >
<!ELEMENT publish_date (#PCDATA) >
<!ELEMENT description (#PCDATA) >
```

BindOTron generates 8 classes. The 'top-level' class is catalog:

```c++
#ifndef BINDOTRON_HEADER_GUARD_catalog
#define BINDOTRON_HEADER_GUARD_catalog

#include <string>
#include <vector>
#include <SAX/XMLReader.h>
#include <SAX/helpers/DefaultHandler.h>
#include "book.h"

class catalog_impl;

class catalog
{
  public:
    catalog();
    catalog(const catalog& rhs);
    ~catalog();

    catalog& operator=(const catalog& rhs);
    bool operator==(const catalog& rhs) const;

    ////////////////
    // getters
    const std::vector& get_book() const;

    ////////////////
    // setters
    void add_book(const book& new_book);
    void remove_book(const book& old_book);

    ////////////////
    // emit_xml
    void emit_xml(std::ostream& os) const;

  private:
    void set_impl(catalog_impl* impl);
    catalog_impl* impl_;
}; // class catalog

class catalog_factory : public SAX::DefaultHandler
{
  public:
    static catalog read(SAX::InputSource& is);

  private:
    catalog_factory(SAX::XMLReader& reader);
    ~catalog_factory();

    const catalog& get_catalog() const;

  private:
    virtual void startElement(const std::string& namespaceURI, const std::string & localName, const std::string& qName, const SAX::Attributes& atts);
    virtual void endElement(const std::string& namespaceURI, const std::string&localName, const std::string& qName);
    virtual void characters(const std::string& ch);

    virtual void fatalError(const SAX::SAXException& exception);

    catalog catalog_;
    SAX::XMLReader& parser_;
    SAX::ContentHandler* child_factory_;
}; // class catalog_factory
#endif
```

Given an XML document conforming to the DTD, creating and populating a catalog and all its sub-objects is

```
  SAX::InputSource is("http://example.com/path/to/catalog");
  catalog my_cat = catalog_factory.read(is);
```