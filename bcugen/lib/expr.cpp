#include <stdio.h>
#include "expr.h"
#include "map.h"

Expr::Expr ()
{
  op1 = 0;
  op2 = 0;
  Type = E_NONE;
  f = 0;
  i = 0;
}

Expr::~Expr ()
{
  if (op1)
    delete op1;
  if (op2)
    delete op2;
}

String
printExpression (Expr * s)
{
  char buf[200];
  String s1;
  if (!s)
    return "???";
  switch (s->Type)
    {
    case Expr::E_AND:
      return (String) "(" + printExpression (s->op1) + "&&" +
	printExpression (s->op2) + ")";
    case Expr::E_OR:
      return (String) "(" + printExpression (s->op1) + "||" +
	printExpression (s->op2) + ")";
    case Expr::E_NOT:
      return (String) "(!" + printExpression (s->op1) + ")";
    case Expr::E_LE:
      return (String) "(" + printExpression (s->op1) + "<=" +
	printExpression (s->op2) + ")";
    case Expr::E_LT:
      return (String) "(" + printExpression (s->op1) + "<" +
	printExpression (s->op2) + ")";
    case Expr::E_GT:
      return (String) "(" + printExpression (s->op1) + ">" +
	printExpression (s->op2) + ")";
    case Expr::E_GE:
      return (String) "(" + printExpression (s->op1) + ">=" +
	printExpression (s->op2) + ")";
    case Expr::E_EQ:
      return (String) "(" + printExpression (s->op1) + "==" +
	printExpression (s->op2) + ")";
    case Expr::E_NE:
      return (String) "(" + printExpression (s->op1) + "!=" +
	printExpression (s->op2) + ")";
    case Expr::E_NOTNULL:
      return (String) "(" + printExpression (s->op1) + "!=0)";
    case Expr::E_PLUS:
      return (String) "(" + printExpression (s->op1) + "+" +
	printExpression (s->op2) + ")";
    case Expr::E_MINUS:
      return (String) "(" + printExpression (s->op1) + "-" +
	printExpression (s->op2) + ")";
    case Expr::E_NEG:
      return (String) "(-" + printExpression (s->op1) + ")";
    case Expr::E_MUL:
      return (String) "(" + printExpression (s->op1) + "*" +
	printExpression (s->op2) + ")";
    case Expr::E_DIV:
      return (String) "(" + printExpression (s->op1) + "/" +
	printExpression (s->op2) + ")";
    case Expr::E_MOD:
      return (String) "(" + printExpression (s->op1) + "%" +
	printExpression (s->op2) + ")";
    case Expr::E_INT:
      sprintf (buf, "%d", s->i);
      return buf;
    case Expr::E_FLOAT:
      sprintf (buf, "%f", s->f);
      return buf;
    case Expr::E_STRING:
      return (String) "\"" + escapeString (s->s) + "\"";
    case Expr::E_PAR:
      return s->s;
    case Expr::E_IN:
      s1 = s->s + " IN(";
      for (int i = 0; i < s->id (); i++)
	s1 += s->id[i] + ",";
      s1 += ")";
      return s1;
    case Expr::E_NONE:
      return "??";
      ;
    }
}
