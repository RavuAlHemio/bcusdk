#ifndef EXPR_H
#define EXPR_H

#include "common.h"

class Expr
{

public:
  Expr ();
  virtual ~ Expr ();

  Expr *op1, *op2;
  ftype f;
  itype i;
  String s;
  IdentArray id;
  enum
  {
    E_AND, E_OR, E_NOT, E_LE, E_LT, E_GT, E_GE, E_EQ, E_NE, E_NOTNULL, E_IN,
    E_PLUS, E_MINUS, E_NEG, E_MUL, E_DIV, E_MOD, E_INT, E_FLOAT, E_STRING,
    E_PAR, E_NONE,
  } Type;

};

String printExpression (Expr * s);

#endif
