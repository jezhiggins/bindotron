#ifndef BINDOTRON_JAVA_GENERATOR_H
#define BINDOTRON_JAVA_GENERATOR_H

#include "Generator.h"

namespace BindOTron
{
  class JavaGenerator : public Generator
  {
    public:
      JavaGenerator(const Model& model) :
          Generator(model) { }

    private:
      virtual void do_generate() const;
  }; // class JavaGenerator
} // namespace BindOTron

#endif