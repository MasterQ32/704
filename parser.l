%{
#include <stdio.h>
#include "tokens.hpp"

#define TOKEN(X) return int(TokenType::X);

%}

STRING	\"[^\"]*\"
COMMENT	;[^\r\n]*
HEXNUM	\$[A-Fa-f0-9]+
NUM	[0-9]+
DIR	\.[A-Za-z0-9]+
ID	[A-Za-z0-9_]+
LBRK	[\n\r]
WS	[ \t]
COMMA	,

%%
{COMMENT}	/* om nom nom */
{HEXNUM}	{ TOKEN(HexNumber); }
{NUM}	{ TOKEN(Number); }
{ID}\:	{ TOKEN(Label); }
{DIR}	{ TOKEN(Directive); }
{ID}{WS}(D|L|R)	{ TOKEN(Name) }
{ID}	{ TOKEN(Name) }
{LBRK}	{ TOKEN(LineBreak); }
{COMMA}	{ TOKEN(Comma); }
{STRING}	{ TOKEN(String); }
\'.\'	{ TOKEN(Character); }
{WS}+	/* om nom nom */
.	{ fprintf(stderr, "line %d: unknown character!", yylineno); }
%%

Token getNextToken()
{
	Token tok;
	tok.type = TokenType(parser_lex());
	tok.line = yylineno;
	tok.text = yytext;
	return tok;
}