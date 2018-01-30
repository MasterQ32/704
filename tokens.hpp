#pragma once

#include <string>

enum class TokenType
{
	EndOfFile = 0,
	HexNumber,
	Number,
	Label,
	Directive,
	Name,
	LineBreak,
	Comma
};

struct Token
{
	TokenType type;
	int line;
	std::string text;
	
	void next();
};

Token getNextToken();