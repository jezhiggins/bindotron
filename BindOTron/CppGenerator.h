#ifndef BINDOTRON_CPP_GENERATOR_H
#define BINDOTRON_CPP_GENERATOR_H

#include "Generator.h"

namespace BindOTron
{
  class CppGenerator : public Generator
  {
    public:
      CppGenerator(const Model& model) :
          Generator(model) { }

    private:
      virtual void do_generate() const;
  }; // class Generator
} // namespace BindOTron

#endif