#ifndef BINDOTRON_MODEL_H
#define BINDOTRON_MODEL_H

#include <string>

namespace BindOTron
{
  class Element;

  class Model
  {
    public: 
      Model();
      virtual ~Model();

      const Element& rootElement() const;

    private:
      virtual const Element& do_rootElement() const = 0;

      // no impls
      Model(const Model&);
      bool operator==(const Model&) const;
      Model& operator=(const Model&);
  }; // class Model
} // namespace BindOTron

#endif