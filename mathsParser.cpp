
#include <iostream>
#include <regex>
#include <string>

class Token
{
public:
    enum class Type {
        kPlus,
        kMinus,
        kMult,
        kDiv,
        kNum,
        kLParen,
        kRParen,
        kEnd,
        kUnknown
    };
    Token() :
      type_(Type::kUnknown),
      value_(0)
    {
    }

    Type GetType()
    {
      return type_;
    }

    void SetType(Token::Type type)
    {
      type_ = type;
    }

    double GetValue()
    {
      return value_;
    }

    void SetValue(double value)
    {
      value_ = value;
    }

private:
    Type type_;
    double value_;
};

class Lexer
{
public:
    /**
    * @brief Takes an input string and can generate the correct series of tokens
    */
    Lexer(std::string input) :
        input_(input),
        get_previous_token_(false),
        previous_token_()
    {
    }

    /**
    * @brief Generates the next token from the input string
    * 
    * The input character that was turned into a token is also removed from the string
    */
    void GetNextToken(Token* token)
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

    /**
    * @brief Forces GenerateNextToken() to pull back the previously used token 
    */
    void revert()
    {
        get_previous_token_ = true;
    }

private:
    std::string input_;
    bool get_previous_token_;

    Token previous_token_;
};

class Parser
{
public:
    /**
    * @brief A recursive parser for simple maths operations
    * 
    * Supports +-/*()
    */
    Parser(std::string input):
        lexer_(input),
        original_input_(input),
        value_(0),
        error_(false)
    {
        current_token_ = new Token();
    }


    /**
    * @brief Returns the computed value
    * 
    * Check there were no errors with error() before using the value
    */
    double GetValue() {
        return value_;
    }

    /**
    * @brief Returns true if any errors were encountered
    */
    bool error() { return error_; }

    /**
    * @brief Returns the origional input
    * 
    * The Lexer eats the first character every time it generates a token
    * and it may simetimes be necessary to get this input back if there 
    * has been an error
    */
    std::string GetInputString() { return original_input_; }

    /**
    * @brief Parses the functin and saves the result
    * 
    * If there are any errors it sets error_ to true
    */
    void parse()
    {
        double expression_value = expression();

        lexer_.GetNextToken(current_token_);
        if (current_token_->GetType() == Token::Type::kEnd && !error_) {
            value_ = expression_value;
        }
        else {
            error_ = true;
        }
    }

private:
    /**
    * @brief Recursively computes an expression
    */
    double expression()
    {
        double component1 = factor();

        lexer_.GetNextToken(current_token_);
        Token::Type current_type = current_token_->GetType();

        while (current_type == Token::Type::kPlus || current_type == Token::Type::kMinus) {
            double component2 = factor();

            if (current_type == Token::Type::kPlus) {
                component1 += component2;
            } else{
                component1 -= component2;
            }

            lexer_.GetNextToken(current_token_);
            current_type = current_token_->GetType();
        }

        lexer_.revert();

        return component1;
    }

    /**
    * @brief Recursively computes an factor
    */
    double factor()
    {
      double factor1 = number();

      lexer_.GetNextToken(current_token_);
      Token::Type current_type = current_token_->GetType();

      while (current_type == Token::Type::kMult || current_type == Token::Type::kDiv) {
          double factor2 = number();

          if (current_type == Token::Type::kMult) {
              factor1 *= factor2;
          }
          else {
              factor1 /= factor2;
          }

          lexer_.GetNextToken(current_token_);
          current_type = current_token_->GetType();
      }
      lexer_.revert();

      return factor1;
    }

    /**
    * @brief Returns a number or ecursively computes an expression if
    * brackets are found
    */
    double number()
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
        }
        else if(current_type == Token::Type::kMinus){
            value = -1.0 * expression();
        } else {
            error_ = true;
        }

        return value;
    }

    Lexer lexer_;
    Token* current_token_;
    std::string original_input_;
    double value_;
    bool error_;
};

int main()
{
    std::cout << "Hello World!\n";
    
    Parser p("5+(3*(5+4)-1.5)");
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