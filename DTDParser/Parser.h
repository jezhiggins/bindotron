#ifndef JEZUK_DTDPARSER_H
#define JEZUK_DTDPARSER_H

// $Id: Parser.h,v 1.18 2001/12/17 15:21:53 jez Exp $

#include <locale>
#include <SAX/ContentHandler.h>
#include <SAX/DTDHandler.h>
#include <SAX/ext/DeclHandler.h>
#include <SAX/ext/LexicalHandler.h>
#include <SAX/ErrorHandler.h>
#include <SAX/InputSource.h>
#include <SAX/helpers/InputSourceResolver.h>
#include <SAX/helpers/StringAdaptor.h>
#include <Utils/convert_adaptor.h>
#include <Utils/utf16utf8codecvt.h>
#include <DTDParser/Tokenizer.h>
#include <DTDParser/DTD.h>
#include <DTDParser/ParameterEntity.h>

namespace DTD 
{

/////////////////////////////////////
template<class stringT, class string_adaptorT = SAX::default_string_adaptor<stringT> >
class Parser
{
  public:
    typedef SAX::basic_ContentHandler<stringT> contentHandlerT;
    typedef SAX::basic_DTDHandler<stringT> dtdHandlerT;
    typedef SAX::basic_DeclHandler<stringT> declHandlerT;
    typedef SAX::basic_LexicalHandler<stringT> lexicalHandlerT;
    typedef SAX::basic_InputSource<stringT> inputSourceT;
    typedef DTD<stringT> dtdT;

    Parser() : 
      contentHandler_(0), 
      dtdHandler_(0), 
      declHandler_(0), 
      lexicalHandler_(0),
      errorHandler_(0) { }
    ~Parser() { }

    void setContentHandler(contentHandlerT& contentHandler) { contentHandler_ = &contentHandler; }
    contentHandlerT* getContentHandler() const { return contentHandler_; };
    void setDTDHandler(dtdHandlerT& dtdHandler) { dtdHandler_ = &dtdHandler; }
    dtdHandlerT* getDTDHandler() const { return dtdHandler_; };
    void setDeclHandler(declHandlerT& declHandler) { declHandler_ = &declHandler; }
    declHandlerT* getDeclHandler() const { return declHandler_; };
    void setLexicalHandler(lexicalHandlerT& lexicalHandler) { lexicalHandler_ = &lexicalHandler; }
    lexicalHandlerT* getLexicalHandler() const { return lexicalHandler_; };
    void setErrorHandler(SAX::ErrorHandler& errorHandler) { errorHandler_ = &errorHandler; }
    SAX::ErrorHandler* getErrorHandler() const { return errorHandler_; };

    void parse(inputSourceT& is);

    const dtdT& DTD() const { return dtd_; }


  private:
    contentHandlerT* contentHandler_;
    dtdHandlerT* dtdHandler_;
    declHandlerT* declHandler_;
    lexicalHandlerT* lexicalHandler_;
    SAX::ErrorHandler* errorHandler_;
    Tokenizer<stringT, string_adaptorT> tokenizer_;
    std::vector<ParameterEntity<stringT> > parameterEntities_;

    typedef Element<stringT> ElementT;
    typedef Attribute<stringT> AttributeT;
    typedef ContentModel<stringT> ContentModelT;
    typedef Token<stringT> TokenT;
    typedef Arabica::Unicode<typename stringT::value_type> UnicodeT;
    typedef ParameterEntity<stringT> ParameterEntityT;

    dtdT dtd_;

    void want(typename TokenT::Type tok);
    void reportError(const std::string& message, bool fatal) const;

    void startDeclaration();
    void elementDeclaration();
    typename ContentModelT::Type elementAnyOrEmpty();
    void elementContent(ContentModelT& cm);
    void elementChildContent(ContentModelT& cm);
    void elementSequenceOrChoice(ContentModelT& cm);
    void elementElement(ContentModelT& cm, const TokenT& t);
    void elementCardinality(ContentModelT& cm);
    void attlistDeclaration();
    AttributeT attlistAttribute();
    void attlistAttributeType(AttributeT& att);
    void attlistAttributeEnumeration(AttributeT& a);
    void attlistAttributeDefault(AttributeT& a);
    void entityDeclaration();
    void parameterEntityDeclaration();
    void notationDeclaration();
    void comment(const stringT& commentText);
    void processingInstruction();

    template<class typeT>
    void readExternalId(typeT& t)
    {
      want(TokenT::IDENTIFIER);
      TokenT tok = tokenizer_.token();
      if(tok.value() == "SYSTEM")
      {
        want(TokenT::QUOTED_STRING);
        t.set_systemId(tokenizer_.token().value());
      }
      else if(tok.value() == "PUBLIC")
      {
        want(TokenT::QUOTED_STRING);
        t.set_publicId(tokenizer_.token().value());
        if(tokenizer_.peek_token() == TokenT::QUOTED_STRING)
          t.set_systemId(tokenizer_.token().value());
      }
      else
        throw std::runtime_error("readExternalId: Expected SYSTEM or PUBLIC");
    } // readExternalId

    ElementT& getElement(const stringT& name);
    
    // no impls
    Parser(const Parser&);
    Parser& operator=(const Parser&);
    bool operator==(const Parser&) const;
}; // class Parser



template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::parse(inputSourceT& is)
{
  InputSourceResolver isr(is);
  if(isr.resolve() == 0)
  {
    reportError("Could not resolve XML document", true);
		return;
  } // if(is.resolver() == 0)

  Arabica::convert::iconvert_adaptor<wchar_t, std::char_traits<wchar_t>, char, std::char_traits<char> > adaptor(*isr.resolve());
  adaptor.imbue(std::locale(adaptor.getloc(), new Arabica::convert::utf16utf8codecvt()));

  try
  {
    tokenizer_.set_istream(adaptor);
    TokenT token;
    while((token = tokenizer_.token()) != TokenT::END)
    {
      switch(token.type())
      {
        case TokenT::START_DECL:
          startDeclaration();
          break;
        case TokenT::COMMENT:
          comment(token.value());
          break;
        case TokenT::START_PI:
          processingInstruction();
          break;
        default:
          throw std::runtime_error("unexpected token");
      } // switch(token)
    } // while
  }
  catch(std::exception& e)
  {
    reportError(e.what(), true);
    return;
  } // catch

  dtd_.validate();
  return;
} // parse

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::want(typename TokenT::Type tok)
{
  if(tokenizer_.peek_token() != tok)
    throw std::runtime_error("unexpected token");
} // want

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::reportError(const std::string& message, bool fatal) const
{
  if(!errorHandler_)
    return;
  
  SAX::SAXParseException e(message);

  if(fatal)
    errorHandler_->fatalError(e);
  else
    errorHandler_->error(e);
} // reportError

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::startDeclaration()
{
  want(TokenT::IDENTIFIER);

  TokenT token = tokenizer_.token();

  if(token.value() == "ELEMENT")
    elementDeclaration();
  else if(token.value() == "ATTLIST")
    attlistDeclaration();
  else if(token.value() == "ENTITY")
    entityDeclaration();
  else if(token.value() == "NOTATION")
    notationDeclaration();
  else 
    throw std::runtime_error("startDeclaration: unknown declaration");
} // startDeclaration

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::elementDeclaration()
{
  want(TokenT::IDENTIFIER);

  ElementT& elem = getElement(tokenizer_.token().value());
  if(elem.content_model() != ContentModelT::UNDEFINED)
    throw std::runtime_error("elementDeclaration: Redefining element");

  TokenT t = tokenizer_.peek_token();
  if(t == TokenT::IDENTIFIER)
    elem.set_content_model(elementAnyOrEmpty());
  else
  {
    ContentModelT cm;
    elementContent(cm);
    elem.set_content_model(cm);
  } // if(t == TokenT::IDENTIFIER)

  if(tokenizer_.token() != TokenT::GT)
    throw std::runtime_error("elementDeclaration: expected >");

  if(declHandler_)
  {
    std::ostringstream s;
    s << elem.content_model();

    declHandler_->elementDecl(elem.name(), s.str());
  } // if(declHandler_)
} // elementDeclaration

template<class stringT, class string_adaptorT>
Parser<stringT, string_adaptorT>::ContentModelT::Type Parser<stringT, string_adaptorT>::elementAnyOrEmpty()
{
  TokenT t = tokenizer_.token();

  if(t.value() == "ANY")
    return ContentModelT::ANY;
  if(t.value() == "EMPTY")
    return ContentModelT::EMPTY;

  throw std::runtime_error("elementAnyOrEmpty : expected ANY or EMPTY");
} // elementAnyOrEmpty

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::elementContent(ContentModelT& cm)
{
  TokenT t = tokenizer_.token();
  if(t != TokenT::LEFT_PARENS)
    throw std::runtime_error("elementContent: expected (");

  t = tokenizer_.peek_token();
  if(t == TokenT::LEFT_PARENS)
    elementChildContent(cm);
  else if((t == TokenT::HASH_MARK) || (t == TokenT::IDENTIFIER))
    elementSequenceOrChoice(cm);
  else
    throw std::runtime_error("elementContent: expected ( or identifier");

  t = tokenizer_.token();
  if(t != TokenT::RIGHT_PARENS)
    throw std::runtime_error("elementContent: expected )");

  elementCardinality(cm);
} // elementContent

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::elementChildContent(ContentModelT& cm)
{
  ContentModelT child_cm;
  elementContent(child_cm);
  cm.add_item(child_cm);
} // elementChildContent

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::elementSequenceOrChoice(ContentModelT& cm)
{
  TokenT t = tokenizer_.token();
  if(t == TokenT::HASH_MARK)
  {
    want(TokenT::IDENTIFIER);
    t = tokenizer_.token();
  } // if ... 

  if(t.value() == "PCDATA")
    cm.add_item(ContentModelT::PCDATA);
  else
    elementElement(cm, t);

  TokenT pt = tokenizer_.peek_token();
  if(pt != TokenT::COMMA && 
     pt != TokenT::PIPE &&
     pt != TokenT::RIGHT_PARENS &&
     pt != TokenT::PLUS &&
     pt != TokenT::QUESTION_MARK &&
     pt != TokenT::ASTERISK)
    throw std::runtime_error("elementSequenceOrChoice: expect , or | or + or * or ? or )");

  while((t = tokenizer_.peek_token()) != TokenT::RIGHT_PARENS)
  {
    if(t == TokenT::LEFT_PARENS)
    {
      elementChildContent(cm);
      continue;
    } // if(TokenT::LEFT_PARENS)

    tokenizer_.token();
    switch(t.type())
    {
      case TokenT::COMMA:
        if(cm.type() == ContentModelT::CHOICE)
          throw std::runtime_error("elementSequenceOrChoice: cannot mix choice and sequences");
        cm.set_type(ContentModelT::SEQUENCE);
        break;
      case TokenT::PIPE:
        if(cm.type() == ContentModelT::SEQUENCE)
          throw std::runtime_error("elementSequenceOrChoice: cannot mix choice and sequences");
        cm.set_type(ContentModelT::CHOICE);
        break;
      case TokenT::HASH_MARK:
        want(TokenT::IDENTIFIER);
        t = tokenizer_.token();
        // fall through
      case TokenT::IDENTIFIER:
        if(t.value() == "PCDATA")
          throw std::runtime_error("elementSequenceOrChoice: #PCDATA must be first in sequence or choice");
        elementElement(cm, t);
        break;
      default:
        throw std::runtime_error("elementSequenceOrChoice: unexpected token");
    } // switch
  } // while

  if(cm.type() == ContentModelT::UNDEFINED)
    cm.set_type(ContentModelT::SEQUENCE);
} // elementSequenceOrChoice

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::elementElement(ContentModelT& cm, const TokenT& t)
{
  ContentModelT child_cm(t.value());
    
  getElement(t.value());
  elementCardinality(child_cm);

  cm.add_item(child_cm);
} // elementElement

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::elementCardinality(ContentModelT& cm)
{
  TokenT t = tokenizer_.peek_token();
  ContentModelT::Cardinality c = ContentModelT::string_to_cardinality(t.value());
  if(c != ContentModelT::UNKNOWN_CARDINALITY)
  {
    cm.set_cardinality(c);
    tokenizer_.token();
  } // if ... 
} // elementCardinality

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::attlistDeclaration()
{
  want(TokenT::IDENTIFIER);

  ElementT& elem = getElement(tokenizer_.token().value());

  do
  {
    AttributeT a = attlistAttribute();

    if(declHandler_)
    {
      declHandler_->attributeDecl(elem.name(),
                                  a.name(),
                                  AttributeT::type_to_string(a.type()),
                                  AttributeT::default_value_to_string(a.default_value()),
                                  a.value());
    } // if(declHandler_)
    
    elem.add_attribute(a);
  }
  while(tokenizer_.peek_token() != TokenT::COMMA && tokenizer_.peek_token() != TokenT::GT);

  if(tokenizer_.token() != TokenT::GT)
    throw std::runtime_error("attlistDeclaration: expected >");

} // attlistDeclaration

template<class stringT, class string_adaptorT>
typename Parser<stringT, string_adaptorT>::AttributeT Parser<stringT, string_adaptorT>::attlistAttribute()
{
  AttributeT a;

  want(TokenT::IDENTIFIER);
  a.set_name(tokenizer_.token().value());

  attlistAttributeType(a);
  attlistAttributeDefault(a);

  return a;
} // attlistAttribute

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::attlistAttributeType(AttributeT& a)
{
  TokenT t = tokenizer_.token();

  if(t == TokenT::IDENTIFIER)
  {
    AttributeT::Type type = AttributeT::string_to_type(t.value());

    a.set_type(type);

    if(type == AttributeT::NOTATION)
    {
      if(tokenizer_.token() != TokenT::LEFT_PARENS)
        throw std::runtime_error("attlistAttributeType: expected (");
      attlistAttributeEnumeration(a);
    } // ...
    if(type == AttributeT::UNKNOWN_TYPE)
      throw std::runtime_error("attlistAttributeType: unknown type");
  } 
  else if(t == TokenT::LEFT_PARENS)
  {
    a.set_type(AttributeT::ENUMERATION);
    attlistAttributeEnumeration(a);
  } 
  else
    throw std::runtime_error("attlistAttributeType: unexpected token");
} // attlistAttributeType

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::attlistAttributeEnumeration(AttributeT& a)
{
  want(TokenT::IDENTIFIER);
  a.add_enum_value(tokenizer_.token().value());

  while(tokenizer_.peek_token() != TokenT::RIGHT_PARENS)
  {
    want(TokenT::PIPE);
    tokenizer_.token();
    want(TokenT::IDENTIFIER);
    a.add_enum_value(tokenizer_.token().value());
  } // while

  tokenizer_.token();
} // attlistAttributeEnumeration

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::attlistAttributeDefault(AttributeT& a)
{
  TokenT t = tokenizer_.token();

  if(t == TokenT::HASH_MARK)
  {
    want(TokenT::IDENTIFIER);
    t = tokenizer_.token();
    AttributeT::DefaultValue dv = AttributeT::string_to_default_value(t.value());

    a.set_default_value(dv);

    if(dv == AttributeT::FIXED)
    {
      want(TokenT::QUOTED_STRING);
      a.set_value(tokenizer_.token().value());
    } // if(dv == AttributeT::FIXED)
    if(dv == AttributeT::UNKNOWN_VALUE)
      throw std::runtime_error("attlistAttributeDefault: unknown type");
  } 
  else if(t == TokenT::QUOTED_STRING)
  {
    a.set_default_value(AttributeT::LITERAL);
    a.set_value(t.value());
  } 
  else
    throw std::runtime_error("attlistAttributeDefault: unexpected token");
} // attlistAttributeDefault

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::entityDeclaration()
{
  if(tokenizer_.peek_token() == TokenT::PERCENT)
    parameterEntityDeclaration();
  else
    throw std::runtime_error("don't know how to handle ENTITY yet");
} // entityDeclaration

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::parameterEntityDeclaration()
{
  tokenizer_.token();
  want(TokenT::IDENTIFIER);

  ParameterEntityT param;
  bool internal = true;

  param.set_name(tokenizer_.token().value());
  if(tokenizer_.peek_token() == TokenT::QUOTED_STRING)
    param.set_value(tokenizer_.token().value());
  else
  {
    internal = false;
    throw std::runtime_error("parameterEntityDeclaration : can not yet resolve external parameter entities");
  } // ....

  want(TokenT::GT);
  tokenizer_.token();

  if(std::find_if(parameterEntities_.begin(), parameterEntities_.end(), std::bind2nd(object_has_name<ParameterEntityT, stringT>(), param.name())) == parameterEntities_.end())
  {
    parameterEntities_.push_back(param);
    if(declHandler_)
    {
      stringT name = UnicodeT::PERCENT_SIGN + param.name();
      if(internal)
        declHandler_->internalEntityDecl(name, param.value());
    } // if(declHandler_)
  } // if ...
} // parameterEntityDeclaration

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::notationDeclaration()
{
  want(TokenT::IDENTIFIER);

  TokenT t = tokenizer_.token();
  Notation<stringT> notation;
  notation.set_name(t.value());
  
  readExternalId(notation);

  if(tokenizer_.token() != TokenT::GT)
    throw std::runtime_error("notationDeclaration: expected >");

  dtd_.notations().push_back(notation);

  if(dtdHandler_)
    dtdHandler_->notationDecl(notation.name(), notation.publicId(), notation.systemId());
} // notationDeclaration

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::comment(const stringT& commentText)
{
  if(lexicalHandler_)
    lexicalHandler_->comment(commentText);
} // comment

template<class stringT, class string_adaptorT>
void Parser<stringT, string_adaptorT>::processingInstruction()
{
  want(TokenT::IDENTIFIER);

  TokenT tok = tokenizer_.token();

  if(tok.skippedWhitespace())
    throw std::runtime_error("processingInstruction: whitespace not allowed between <? and target name");

  stringT target = tok.value();
  string_adaptorT SA;
  if((target[0] == SA.makeValueT('X') || target[0] == SA.makeValueT('x')) &&
     (target[1] == SA.makeValueT('M') || target[1] == SA.makeValueT('m')) &&
     (target[2] == SA.makeValueT('L') || target[2] == SA.makeValueT('l')))
    throw std::runtime_error("processingInstruction: illegal name");

  stringT data;
  while(true)
  {
    data += tokenizer_.readUntil(UnicodeT::QUESTION_MARK);
    tokenizer_.token();
    tok = tokenizer_.token();
    if(tok == TokenT::GT)
      break;

    data += UnicodeT::QUESTION_MARK;
    if(tok.skippedWhitespace())
      data += UnicodeT::SPACE;
    data += tok.value();
  } // while(true)

  if(contentHandler_)
    contentHandler_->processingInstruction(target, data);
} // processingInstruction

template<class stringT, class string_adaptorT>
typename Parser<stringT, string_adaptorT>::ElementT& Parser<stringT, string_adaptorT>::getElement(const stringT& name)
{
  for(dtdT::ElementList::iterator i = dtd_.elements().begin(); i != dtd_.elements().end(); ++i)
    if(i->name() == name)
      return *i;

  ElementT elem;
  elem.set_name(name);
  dtd_.elements().push_back(elem);
  return dtd_.elements().back();
} // getElement

} // namespace DTD    
#endif