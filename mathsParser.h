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
        kPow,
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
  Lexer(std::string input);

  /**
  * @brief Generates the next token from the input string
  *
  * The input character that was turned into a token is also removed from the string
  */
  void GetNextToken(Token* token);

  /**
  * @brief Forces GenerateNextToken() to pull back the previously used token
  */
  void revert();

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
  * Supports ()^/*+-
  */
  Parser(std::string input);

  /**
  * @brief Returns the computed value
  *
  * Check there were no errors with error() before using the value
  */
  double GetValue();

  /**
  * @brief Returns the origional input
  *
  * The Lexer eats the first character every time it generates a token
  * and it may simetimes be necessary to get this input back if there
  * has been an error
  */
  std::string GetInputString() { return original_input_; }

  /**
  * @brief Returns true if any errors were encountered
  */
  bool error() { return error_; }

  /**
  * @brief Parses the functin and saves the result
  *
  * If there are any errors it sets error_ to true
  */
  void parse();

private:
  /**
  * @brief Recursively computes an expression
  */
  double expression();

  /**
  * @brief Recursively computes an factor
  */
  double factor();

  /**
  * @brief Returns a number or ecursively computes an expression if
  * brackets are found
  */
  double number();


    Lexer lexer_;
    Token* current_token_;
    std::string original_input_;
    double value_;
    bool error_;
};