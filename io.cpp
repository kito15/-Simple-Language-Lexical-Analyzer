#include <iostream>
#include <regex>
#include <map>
#include "lex.h"
using namespace std;
static map<Token, string> tokenPrint {{PRINT, "PRINT"}, {IF, "IF"},{BEGIN, "BEGIN"}, {END, "END"},{THEN, "THEN"}, {IDENT, "IDENT"},{ICONST, "ICONST"}, {SCONST, "SCONST"},{RCONST, "RCONST"}, {PLUS, "PLUS"},{MINUS, "MINUS"}, {MULT, "MULT"},{DIV, "DIV"}, {EQ, "EQ"},{LPAREN, "LPAREN"}, {RPAREN, "RPAREN"},{SCOMA, "SCOMA"}, {COMA, "COMA"},{ERR, "ERR"}, {DONE, "DONE"}
};
inline ostream & operator << (ostream & out,const LexItem & tok) {
  Token token_ = tok.GetToken();
  string * token = & tokenPrint[token_];
  cout << * token;
  bool value = (token_ == SCONST || token_ == RCONST ||token_ == ICONST || token_ == IDENT ||token_ == ERR);
  if (value==true)
    cout << " (" << tok.GetLexeme() << ")";
  return out;
}
inline LexItem currentToken;
inline LexItem previousToken;

inline LexItem getNextToken(istream & in , int & linenum) {
  enum TokenState {START,INID,STRING,ININT,REAL,INCOMMENT,SIGN}
  lexstate = START;
  string lexeme;
  char ch;

  while (in.get(ch)) {
    switch (lexstate) {  // The state of searching for a token
    case ININT:
      if (isalpha(ch)) //checking to see if a alphanumeric character is in front of an integer number
        return LexItem(ERR, lexeme + ch, linenum);
      if (regex_match(lexeme + ch, regex("[0-9]+"))) {
        lexeme += ch;
      } else if (ch == '.') {
        lexstate = REAL; in .putback(ch);
        continue;
      } else {
        lexstate = START; in .putback(ch);
        currentToken = LexItem(ICONST, lexeme, linenum);
        previousToken = currentToken;
        return currentToken;
      }
      break;
    case REAL:
      if (isalpha(ch))    // checking to see if an alphanumeric character is in front of an real number
        return LexItem(ERR, lexeme + ch, linenum);
      if (regex_match(lexeme + ch, regex("[0-9]*\\.[0-9]+"))) {
        lexeme += ch;
      } else if (regex_match(lexeme + ch, regex("[0-9]*\\.[0-9]*"))) {
        lexeme += ch;
      } else {
        if (lexeme[lexeme.length() - 1] == '.')
          return LexItem(ERR, lexeme, linenum);
        lexstate = START; in .putback(ch);
        currentToken = LexItem(RCONST, lexeme, linenum);
        previousToken = currentToken;
        return currentToken;
      }
      break;
    case STRING:
      if (ch == 10) //returns an error if the string contains multiple lines
        return LexItem(ERR, lexeme, linenum);
      if (regex_match(lexeme + ch, regex("\"[ -~]*"))) {
        if (ch == '\\' && in.peek() == '\"') {
          lexeme += ch; in .get(ch);
          lexeme += ch;
          continue;
        } else
          lexeme += ch;
      }
      if (regex_match(lexeme + ch, regex("\"[ -~]*\""))) {
        lexstate = START;
        currentToken = LexItem(SCONST, lexeme, linenum);
        previousToken = currentToken;
        return currentToken;
      }
      break;

    case START:
      if (ch == '\n')
        linenum++;

      if ( in .peek() == -1) {  //If end of file is hit stops searching
        return LexItem(DONE, lexeme, linenum);
      }
      if (isspace(ch))
        continue;

      lexeme = ch;
      if (ch == '/' && char( in .peek()) == '/') {
        lexstate = INCOMMENT;
        continue;
      }
      if (ch == '+' || ch == '-' || ch == '*' ||ch == '/' || ch == '(' || ch == ')' ||ch == '=' || ch == ',' || ch == ';') {  //checks for signs such as RPAR OR LPAR
        lexstate = SIGN;
        continue;
      }
      if (ch == '\"') {  //checks for string
        lexstate = STRING;
        continue;
      }
      if (isdigit(ch)) { //checks to see if int
        lexstate = ININT;
        continue;
      }
      if (ch == '.') {
        lexstate = REAL;
        continue;
      }
      if (isalpha(ch)) {
        lexstate = INID;
        continue;
      }
      return LexItem(ERR, lexeme, linenum); // returns an error if none of the above characters are recognized.

    case INID:
      if (regex_match(lexeme + ch, regex("[a-zA-Z][a-zA-Z0-9]*"))) //matches strings and properly formats them
        lexeme += ch;
      if ( in .peek() == -1 || !regex_match(lexeme + ch, regex("[a-zA-Z][a-zA-Z0-9]*"))) {
        lexstate = START; in .putback(ch);
        if (lexeme == "begin") {
          if (previousToken.GetToken() != ERR)
            return LexItem(ERR, lexeme, linenum);
          currentToken = LexItem(BEGIN, lexeme, linenum);
        } else if (lexeme == "print")
          currentToken = LexItem(PRINT, lexeme, linenum);
        else if (lexeme == "end") {
          if (previousToken.GetToken() != SCOMA)
            return LexItem(ERR, previousToken.GetLexeme(), linenum);
          currentToken = LexItem(END, lexeme, linenum);
        } else if (lexeme == "if")
          currentToken = LexItem(IF, lexeme, linenum);
        else if (lexeme == "then")
          currentToken = LexItem(THEN, lexeme, linenum);
        else {
          if (previousToken.GetToken() == IDENT)
            return LexItem(ERR, lexeme, linenum);
          currentToken = LexItem(IDENT, lexeme, linenum);
        }
        if (currentToken != BEGIN && previousToken == ERR)
          return LexItem(ERR, "No BEGIN Token", currentToken.GetLinenum());
        previousToken = currentToken;
        return currentToken;
      }
      break;
    case INCOMMENT: //ignores any comments
      if (ch == '\n') {
        linenum++;
        lexstate = START;
      }
      continue;

    case SIGN: //Check for begin token at the beggining
      if (previousToken == ERR)
        return LexItem(ERR, "No Begin Token", linenum);
      if (lexeme == "+" || lexeme == "*" || lexeme == "/") {
        Token token = previousToken.GetToken();
        if (token == IDENT || token == ICONST || token == RCONST) {
          lexstate = START; in .putback(ch);
          if (lexeme == "+")
            currentToken = LexItem(PLUS, lexeme, linenum);
          else if (lexeme == "*")
            currentToken = LexItem(MULT, lexeme, linenum);
          else
            currentToken = LexItem(DIV, lexeme, linenum);
          previousToken = currentToken;
          return currentToken;
        } else
          return LexItem(ERR, lexeme + ch, linenum);
      }
      if (lexeme == "-") {
        Token token = previousToken.GetToken();
        if (token == IDENT || token == ICONST || token == RCONST || token == EQ) {
          lexstate = START; in .putback(ch);
          currentToken = LexItem(MINUS, lexeme, linenum);
          previousToken = currentToken;
          return currentToken;
        } else
          return LexItem(ERR, lexeme + ch, linenum);
      }
      if (lexeme == "(") {
        Token token = previousToken.GetToken();
        if (token == IF || token == EQ || token == PLUS || token == MINUS ||
          token == MULT || token == DIV) {
          lexstate = START; in .putback(ch);
          currentToken = LexItem(LPAREN, lexeme, linenum);
          previousToken = currentToken;
          return currentToken;
        } else
          return LexItem(ERR, lexeme + ch, linenum);
      }
      if (lexeme == ")") {
        Token token = previousToken.GetToken();
        if (token == ICONST || token == RCONST || token == IDENT) {
          lexstate = START; in .putback(ch);
          currentToken = LexItem(RPAREN, lexeme, linenum);
          previousToken = currentToken;
          return currentToken;
        } else
          return LexItem(ERR, lexeme + ch, linenum);
      }
      if (lexeme == "=") {
        Token token = previousToken.GetToken();
        if (token == IDENT) {
          lexstate = START; in .putback(ch);
          currentToken = LexItem(EQ, lexeme, linenum);
          previousToken = currentToken;
          return currentToken;
        } else
          return LexItem(ERR, lexeme + ch, linenum);
      }
      if (lexeme == ",") {
        Token token = previousToken.GetToken();
        if (token == SCONST) {
          lexstate = START; in .putback(ch);
          currentToken = LexItem(COMA, lexeme, linenum);
          previousToken = currentToken;
          return currentToken;
        } else
          return LexItem(ERR, lexeme + ch, linenum);
      }
      if (lexeme == ";") {
        Token token = previousToken.GetToken();
        if (token == SCONST || token == ICONST || token == RCONST || token == IDENT) {
          lexstate = START; in .putback(ch);
          currentToken = LexItem(SCOMA, lexeme, linenum);
          previousToken = currentToken;
          return currentToken;
        } else
          return LexItem(ERR, lexeme + ch, linenum);
      }
      break;
    }
  }
}
