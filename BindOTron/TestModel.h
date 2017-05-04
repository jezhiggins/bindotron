#ifndef BINDOTRON_TEST_MODEL_H
#define BINDOTRON_TEST_MODEL_H

#include <string>
#include <map>
#include "Model.h"
#include "Element.h"

namespace BindOTron
{
  class TestModel : public Model
  {
    public: 
      TestModel();
      ~TestModel();

    private:
      virtual const Element& do_rootElement() const;

      Element rootElement;
  }; // class TestModel
} // namespace BindOTron

#endif