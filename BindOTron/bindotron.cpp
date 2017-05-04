
#pragma warning(disable: 4786)
#include "TestModel.h"
#include "DTDModel.h"
#include "CppGenerator.h"
#include "JavaGenerator.h"
#include <SAX/InputSource.h>
#include <iostream>

int main(int argc, const char* argv[])
{
  if(argc != 2)
  {
    std::cout << "BINDOTRON DTD_filename" << std::endl;
    return 0;
  } // if(argc != 2)

  try
  {
    //BindOTron::TestModel model;
    SAX::InputSource input(argv[1]);
    BindOTron::DTDModel model;

    model.loadDTD(input);

    BindOTron::CppGenerator generator(model);
    generator.generate();
    //BindOTron::JavaGenerator jgenerator(model);
    //jgenerator.generate();
  } // try
  catch(const std::exception& ex)
  {
    std::cout << "Exception:" << ex.what() << std::endl;
  } // catch

  return 0;
} // main

// end of file