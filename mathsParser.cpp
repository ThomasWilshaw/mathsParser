
#include "mathsParser.h"

Lexer::Lexer(std::string input) :
    input_(input),
    get_previous_token_(false),
    previous_token_()
{
}

void Lexer::GetNextToken(Token* token)
{
  if (get_previous_token_) {
    get_previous_token_ = false;
    token->SetType(previous_token_.GetType());
    token->SetValue(previous_token_.GetValue());
    return;
  }
  token->SetType(Token::Type::kUnknown);

  // Use QString to strip leading white spaces
  if (std::regex_match(input_, std::regex(R"(^\+.*)"))) {
    token->SetType(Token::Type::kPlus);
    input_.erase(0, 1);
  } else if (std::regex_match(input_, std::regex(R"(^\-.*)"))) {
    token->SetType(Token::Type::kMinus);
    input_.erase(0, 1);
  } else if (std::regex_match(input_, std::regex(R"(^\*.*)"))) {
    token->SetType(Token::Type::kMult);
    input_.erase(0, 1);
  }else if (std::regex_match(input_, std::regex(R"(^\/.*)"))) {
    token->SetType(Token::Type::kDiv);
    input_.erase(0, 1);
  } else if (std::regex_match(input_, std::regex(R"(^\^.*)"))) {
    token->SetType(Token::Type::kPow);
    input_.erase(0, 1);
  } else if (std::regex_match(input_, std::regex(R"(^\d+\.?\d*.*)"))) {
    token->SetType(Token::Type::kNum);
    std::smatch m;
    std::regex_search(input_, m, std::regex(R"(^\d+\.?\d*.*)"));
    token->SetValue(stod(m[0].str()));
    input_ = std::regex_replace(input_, std::regex(R"(^\d+\.?\d*)"), "", std::regex_constants::format_first_only);
  } else if (std::regex_match(input_, std::regex(R"(^\(.*)"))) {
    token->SetType(Token::Type::kLParen);
    input_.erase(0, 1);
  } else if (std::regex_match(input_, std::regex(R"(^\).*)"))) {
    token->SetType(Token::Type::kRParen);
    input_.erase(0, 1);
  } else if (input_.empty()) {
    token->SetType(Token::Type::kEnd);
  }

  if (token->GetType() == Token::Type::kUnknown) {
    input_ = ""; // Not ideal but forces computation to stop
  }


  // Saves this token in case we need to go backwards later
  previous_token_.SetType(token->GetType());
  previous_token_.SetValue(token->GetValue());
}

void Lexer::revert()
{
  get_previous_token_ = true;
}



Parser::Parser(std::string input):
        lexer_(input),
        original_input_(input),
        value_(0),
        error_(false)
{
  current_token_ = new Token();
}

double Parser::GetValue() {
  return value_;
}

void Parser::parse()
{
  double expression_value = expression();

  lexer_.GetNextToken(current_token_);
  if (current_token_->GetType() == Token::Type::kEnd && !error_) {
    value_ = expression_value;
  } else {
    error_ = true;
  }
}

double Parser::expression()
{
  double component1 = factor();

  lexer_.GetNextToken(current_token_);
  Token::Type current_type = current_token_->GetType();

  while (current_type == Token::Type::kPlus || current_type == Token::Type::kMinus) {
    double component2 = factor();

    if (current_type == Token::Type::kPlus) {
      component1 += component2;
    } else {
      component1 -= component2;
    }

    lexer_.GetNextToken(current_token_);
    current_type = current_token_->GetType();
  }

  lexer_.revert();

  return component1;
}


double Parser::factor()
{
  double factor1 = number();

  lexer_.GetNextToken(current_token_);
  Token::Type current_type = current_token_->GetType();

  while (current_type == Token::Type::kMult || current_type == Token::Type::kDiv || current_type == Token::Type::kPow) {
    double factor2 = number();

    if (current_type == Token::Type::kMult) {
      factor1 *= factor2;
    } else if(current_type == Token::Type::kDiv) {
      factor1 /= factor2;
    } else {
      factor1 = pow(factor1, factor2);
    }

    lexer_.GetNextToken(current_token_);
    current_type = current_token_->GetType();
  }
  lexer_.revert();

  return factor1;
}

    
double Parser::number()
{
  lexer_.GetNextToken(current_token_);

  Token::Type current_type = current_token_->GetType();
  double value = 0;

  if (current_type == Token::Type::kLParen) {
    value = expression();

    lexer_.GetNextToken(current_token_);
    if (current_token_->GetType() != Token::Type::kRParen) {
      error_ = true;
    }
  } else if (current_type == Token::Type::kNum) {
    value = current_token_->GetValue();
  } else if(current_type == Token::Type::kMinus){
    value = -1.0 * expression();
  } else {
    error_ = true;
  }
  
  return value;
}

    


int main()
{
    std::cout << "Hello World!\n";
    
    Parser p("(5-3)^-(2+2)");
    p.parse();
    if (!p.error()) {
        std::cout << p.GetValue();
    }
    else {
        std::cout << "ERROR";
    }
    
    /*
    Useful links:
    https://lukaszwrobel.pl/blog/math-parser-part-3-implementation/
    https://www.softwaretestinghelp.com/regex-in-cpp/
    https://stackoverflow.com/questions/12117024/decimal-number-regular-expression-where-digit-after-decimal-is-optional
    https://www.keycdn.com/support/regex-cheatsheet
    https://stackoverflow.com/questions/12117024/decimal-number-regular-expression-where-digit-after-decimal-is-optional
    https://regexr.com/
    */
}