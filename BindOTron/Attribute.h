#ifndef BINDOTRON_ATTRIBUTE_H
#define BINDOTRON_ATTRIBUTE_H

#include <string>
#include <vector>
#include "Value.h"

namespace BindOTron
{
  class Attribute : public Value
  {
    public:
      Attribute() 
      {
        set_type(Type::STRING);
      } // Attribute
   
    private:
      // no impls
      bool operator==(const Attribute&) const;
  }; // class Attribute
} // namespace BindOTron

#endif














