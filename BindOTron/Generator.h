#ifndef BINDOTRON_GENERATOR_H
#define BINDOTRON_GENERATOR_H

#include "Model.h"

namespace BindOTron
{
  class Generator
  {
    public:
      Generator(const Model& model) :
          model_(model) { }
      virtual ~Generator();

      void generate() const;

    protected:
      const Model& model() const { return model_; }

    private:
      virtual void do_generate() const = 0;

      // instance variables
      const Model& model_;

      // no impl
      Generator(const Generator&);
      bool operator==(const Generator&) const;
      Generator& operator=(const Generator&);
  }; // class Generator
} // namespace BindOTron

#endif