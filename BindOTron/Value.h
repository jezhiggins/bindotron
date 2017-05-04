#ifndef BINDOTRON_VALUE_H
#define BINDOTRON_VALUE_H

#include <string>

namespace BindOTron
{
  class Value
  {
    public:
      enum Type
      {
        STRING,
        LIST,
        INT,
        FLOAT,
        OBJECT
      };

      Value() { }
      Value(const Value& rhs) : 
        type_(rhs.type_),
        name_(rhs.name_){ }
      Value& operator=(const Value& rhs)
      {
        type_ = rhs.type_;
        name_ = rhs.name_;

        return *this;
      } // operator=
      virtual ~Value() { }

      std::string name() const { return name_; }
      Type type() const { return type_; }

      void set_name(const std::string& name) { name_ = name; }
      void set_type(Type type) { type_ = type; }

    private:
      Type type_;
      std::string name_;

      // no impls
      bool operator==(const Value&) const;
  }; // struct Value
} // namespace BindOTron

#endif














