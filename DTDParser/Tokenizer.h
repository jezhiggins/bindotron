#ifndef ARABICA_DTDPARSER_TOKENIZER_H
#define ARABICA_DTDPARSER_TOKENIZER_H

// $Id: Tokenizer.h,v 1.11 2001/12/13 16:12:20 jez Exp $

#include <string>
#include <XML/UnicodeCharacters.h>
#include <XML/XMLCharacterClasses.h>

namespace DTD
{

template<class stringT>
class Token
{
  public:
    enum Type
    {
      COMMA,
      PLUS,
      QUESTION_MARK,
      LEFT_PARENS,
      RIGHT_PARENS,
      PIPE,
      LT,
      GT,
      HASH_MARK,
      EQUALS,
      ASTERISK,
      START_CONDITIONAL,
      END_CONDITIONAL,
      START_DECL,
      COMMENT,
      START_PI,
      PERCENT,
      IDENTIFIER,
      QUOTED_STRING,
      NMTOKEN,
      CHARACTER,
      END
    };
    
    Token() : skipped_(false) { } 
    Token(const Token& rhs) : type_(rhs.type_), value_(rhs.value_), skipped_(rhs.skipped_) { }
    ~Token() { }

    Token& operator=(const Token& rhs)
    {
      type_ = rhs.type_;
      value_ = rhs.value_;
      skipped_ = rhs.skipped_;
      return *this;
    } // operator=

    bool operator==(Type type) const { return type_ == type; } 
    bool operator!=(Type type) const { return !(operator==(type)); }

    Type type() const { return type_; }
    const stringT& value() const { return value_; }
    bool skippedWhitespace() const { return skipped_; }

    void set_type(Type type) { type_ = type; }
    void set_value(const stringT& value) { value_ = value; }
    void set_skipped(bool skipped) { skipped_ = skipped; }

  private:
    Type type_;
    stringT value_;
    bool skipped_;

    bool operator==(const Token&) const; // no impl
}; // class Token

///////////////////////////////////////////////////////////////////
// Tokenizer
template<class stringT, class string_adaptorT>
class Tokenizer
{
  public:
    typedef Token<stringT> TokenT;
    typedef typename Token<stringT>::Type TokenTypeT;

    Tokenizer() : has_next_(false) { }

    void set_istream(std::wistream& istream) { stream_ = &istream; }
    TokenT token();
    TokenT peek_token();

    typedef typename stringT::value_type charT;
    stringT readUntil(charT delim);

  private:
    void readQuotedString(std::istreambuf_iterator<wchar_t>& is, wchar_t quot, std::wstring& tok) const;
    void readToken(std::istreambuf_iterator<wchar_t>& is, std::wstring& tok) const;
    TokenTypeT readStartOfSomething(std::istreambuf_iterator<wchar_t>& is, std::wstring& tok);
    void readEndOfConditional(std::istreambuf_iterator<wchar_t>& is);
    void reportError(const std::string& message) const;

    std::wistream* stream_;
    const std::istreambuf_iterator<wchar_t> end_stream_;

    TokenT next_token_;
    bool has_next_;
    string_adaptorT SA_;

    typedef Arabica::Unicode<typename stringT::value_type> UnicodeT;
}; // class Tokenizer

///////////////////////////////////////
// Tokenizer implementation

template<class stringT, class string_adaptorT>
typename Tokenizer<stringT, string_adaptorT>::TokenT Tokenizer<stringT, string_adaptorT>::token()
{
  if(!has_next_)
    peek_token();

  has_next_ = false;
  return next_token_;
} // token

template<class stringT, class string_adaptorT>
typename Tokenizer<stringT, string_adaptorT>::TokenT Tokenizer<stringT, string_adaptorT>::peek_token()
{
  if(has_next_)
    return next_token_;

  has_next_ = true;
  next_token_ = TokenT();

  std::istreambuf_iterator<wchar_t> is(*stream_);
  if(is == end_stream_)
  {
    next_token_.set_type(TokenT::END);
    return next_token_;
  } //

  wchar_t c = *is++;
  next_token_.set_skipped(Arabica::XML::is_space(c));
  while((Arabica::XML::is_space(c)) && (is != end_stream_))
    c = *is++;

  if(is == end_stream_ && Arabica::XML::is_space(c))
  {
    next_token_.set_type(TokenT::END);
    return next_token_;
  } //

  std::wstring tok;
  tok += c;

  if(c == UnicodeT::LESS_THAN_SIGN)
    next_token_.set_type(readStartOfSomething(is, tok));
  else if(c == UnicodeT::RIGHT_SQUARE_BRACKET)
    readEndOfConditional(is);
  else if((c == UnicodeT::APOSTROPHE) || (c == UnicodeT::QUOTATION_MARK))
  {
    readQuotedString(is, c, tok);
    next_token_.set_type(TokenT::QUOTED_STRING);
  } // if ... 
  else if(Arabica::XML::is_letter(c))
  {
		readToken(is, tok);
		next_token_.set_type(TokenT::IDENTIFIER);
	}
	else if(Arabica::XML::is_name_char(c))
	{
		readToken(is, tok);
		next_token_.set_type(TokenT::NMTOKEN);
	}
  else if(c == UnicodeT::PERCENT_SIGN)
  {
    if(Arabica::XML::is_space(*is))
      next_token_.set_type(TokenT::PERCENT);
    else
      throw std::runtime_error("can't handle parameter entity ref's yet");
  }
  else if(c == UnicodeT::QUESTION_MARK)
    next_token_.set_type(TokenT::QUESTION_MARK);
  else if(c == UnicodeT::LEFT_PARENTHESIS)
    next_token_.set_type(TokenT::LEFT_PARENS);
  else if(c == UnicodeT::RIGHT_PARENTHESIS)
    next_token_.set_type(TokenT::RIGHT_PARENS);
  else if(c == UnicodeT::VERTICAL_BAR)
    next_token_.set_type(TokenT::PIPE);
  else if(c == UnicodeT::GREATER_THAN_SIGN)
    next_token_.set_type(TokenT::GT);
  else if(c == UnicodeT::EQUALS_SIGN)
    next_token_.set_type(TokenT::EQUALS);
  else if(c == UnicodeT::ASTERISK)
    next_token_.set_type(TokenT::ASTERISK);
  else if(c == UnicodeT::PLUS_SIGN)
    next_token_.set_type(TokenT::PLUS);
  else if(c == UnicodeT::COMMA)
    next_token_.set_type(TokenT::COMMA);
  else if(c == UnicodeT::NUMBER_SIGN)
    next_token_.set_type(TokenT::HASH_MARK);
  else
    next_token_.set_type(TokenT::CHARACTER);

/*
		if((c == '&') || (c == '%'))
    {
      if((ch == '%') && XML::is_space(*is))
        return new Token(PERCENT);

      boolean peRef = (ch == '%');
  		StringBuffer buff = new StringBuffer();
			buff.append((char) ch);

			while (isIdentifierChar((char) peekChar()))
			{
				buff.append((char) read());
			}

			if(read() != ';')
			{
           throw new DTDParseException(getUriId(),
                                "Expected ';' after reference "+
                                buff.toString()+", found '"+ch+"'",
                                getLineNumber(), getColumn());
				}
                buff.append(';');

                if (peRef && expandEntity(buff.toString()))
                {
                    continue;
                }
				return new Token(IDENTIFIER, buff.toString());
		}
*/
  next_token_.set_value(SA_.makeStringT(tok.c_str()));
  return next_token_;
} // peek_token


template<class stringT, class string_adaptorT>
stringT Tokenizer<stringT, string_adaptorT>::readUntil(charT delim)
{
  has_next_ = false;

  wchar_t w_delim = SA_.makeValueT(delim);

  std::istreambuf_iterator<wchar_t> is(*stream_);
  std::wstring str;
  while((*is != w_delim) && (is != end_stream_))
    str += *is++;

  return SA_.makeStringT(str.c_str());
} // readUntil

template<class stringT, class string_adaptorT>
void Tokenizer<stringT, string_adaptorT>::readQuotedString(std::istreambuf_iterator<wchar_t>& is, wchar_t quot, std::wstring& tok) const
{
  tok.erase();
  while((*is != quot) && (is != end_stream_))
    tok += *is++;
  is++;

  if(is == end_stream_)
    throw std::runtime_error("unterminated string");
} // readQuotedString

template<class stringT, class string_adaptorT>
void Tokenizer<stringT, string_adaptorT>::readToken(std::istreambuf_iterator<wchar_t>& is, std::wstring& tok) const
{
  while(Arabica::XML::is_name_char(*is))
    tok += *is++;
} // readToken

template<class stringT, class string_adaptorT>
typename Tokenizer<stringT, string_adaptorT>::TokenTypeT Tokenizer<stringT, string_adaptorT>::readStartOfSomething(std::istreambuf_iterator<wchar_t>& is, std::wstring& tok)
{
  if(*is == UnicodeT::EXCLAMATION_MARK)
  {
    is++;
    if(*is == UnicodeT::LEFT_SQUARE_BRACKET)
    {
      is++;
      return TokenT::START_CONDITIONAL;
    }
    if(*is != UnicodeT::HYPHEN_MINUS)
      return TokenT::START_DECL;

    // possible start of comment
    is++;
    if(*is++ != UnicodeT::HYPHEN_MINUS) 
      reportError("Invalid character sequence");

    tok.erase();
    while(true)
    {
      while((Arabica::XML::is_char(*is)) && (*is != UnicodeT::HYPHEN_MINUS) && (is != end_stream_))
        tok += *is++;
      ++is;
      if(*is == UnicodeT::HYPHEN_MINUS)
      {
        ++is;
        if(*is == UnicodeT::GREATER_THAN_SIGN)
        {
          ++is;
          return TokenT::COMMENT;
        } // if(*is == UnicodeT::GREATER_THAN_SIGN)
        throw std::runtime_error("-- is illegal inside a comment");
      }
      if(is == end_stream_)
        throw std::runtime_error("unterminated comment");
      tok += UnicodeT::HYPHEN_MINUS;
    }
  } // if(*is == UnicodeT::EXCLAMATION_MARK

  if(*is == UnicodeT::QUESTION_MARK)
  {
    is++;
    return TokenT::START_PI;
  } // if(*is == UnicodeT::QUESTION_MARK)

  return TokenT::LT;
} // readStartOfSomething

template<class stringT, class string_adaptorT>
void Tokenizer<stringT, string_adaptorT>::readEndOfConditional(std::istreambuf_iterator<wchar_t>& is)
{
  is++;
  if((*is++ != UnicodeT::RIGHT_SQUARE_BRACKET) && (*is != UnicodeT::GREATER_THAN_SIGN))
  {
    reportError("Invalid character " + *is);      
    next_token_.set_type(TokenT::END);
  }
  next_token_.set_type(TokenT::END_CONDITIONAL);
} // readEndOfConditional

template<class stringT, class string_adaptorT>
void Tokenizer<stringT, string_adaptorT>::reportError(const std::string& message) const
{
  throw std::runtime_error(message);
} // reportError

} // namespace DTD
#endif 

