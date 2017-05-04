#ifndef BINDOTRON_ELEMENT_H
#define BINDOTRON_ELEMENT_H

#include <string>
#include <vector>
#include "Value.h"
#include "Attribute.h"

namespace BindOTron
{
  class Element : public Value
  {
    public:
      Element() :
          hasValue_(false),
          attributes_(),
          elements_()
      {
        set_type(Value::LIST);
        value_.set_name("value");
        value_.set_type(Value::STRING);
      } // Element

      Element(const Element& rhs) :
          hasValue_(rhs.hasValue_),
          attributes_(rhs.attributes_),
          elements_(rhs.elements_),
          value_(rhs.value_),
          Value(rhs)
      {
      } // Element

      Element& operator=(const Element& rhs) 
      {
        Value::operator=(rhs);

        hasValue_ = rhs.hasValue_;
        attributes_ = rhs.attributes_;
        elements_ = rhs.elements_;
        value_ = rhs.value_;

        return *this;
      } // operator=

   
      bool has_value() const { return hasValue_; }
      const Value& value() const { return value_; }
      const std::vector<Attribute>& attributes() const { return attributes_; }
      const std::vector<Element>& elements() const { return elements_; }

      void set_value() { hasValue_ = true; }
      void set_value_type(const Value::Type& type) { value_.set_type(type); hasValue_ = true; }
      void add_attribute(const Attribute& name) { attributes_.push_back(name); }
      void add_element(const Element& name) { elements_.push_back(name); }

    private:
      bool hasValue_;
      Value value_;
      std::vector<Attribute> attributes_;
      std::vector<Element> elements_;

      // no impls
      bool operator==(const Element&) const;
  }; // class Element
} // namespace BindOTron

#endif














