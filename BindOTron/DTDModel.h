#ifndef BINDOTRON_DTD_MODEL_H
#define BINDOTRON_DTD_MODEL_H

#include <DTDParser/DTD.h>
#include <SAX/InputSource.h>
#include "Model.h"
#include "Element.h"

namespace BindOTron
{
  class DTDModel : public Model
  {
    public: 
      DTDModel();
      ~DTDModel();

      void loadDTD(SAX::InputSource& inputSource);

    private:
      virtual const Element& do_rootElement() const;
      void populate(Element& elem, const DTD::DTD<std::string>& dtd);
      void populate_content_model(Element& elem, const DTD::ContentModel<std::string>& content_model, const DTD::DTD<std::string>& dtd);
  
      Element rootElement_;
  }; // class DTDModel
} // namespace BindOTron

#endif