
#pragma warning(disable: 4786)
#include "Generator.h"

namespace BindOTron
{
  Generator::~Generator()
  {
  } // ~Generator

  void Generator::generate() const
  {
    do_generate();
  } // generate

} // namespace BindOTron

// end of file