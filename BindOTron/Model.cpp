
#pragma warning(disable: 4786)
#include "Model.h"

namespace BindOTron
{

  Model::Model()
  {
  } // Model

  Model::~Model()
  {
  } // ~Model

  const Element& Model::rootElement() const
  {
    return do_rootElement();
  } // rootElement
} // namespace BindOTron

// end of file