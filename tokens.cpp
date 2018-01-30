#include "tokens.hpp"
#include <cassert>


void Token::next()
{
	*this = getNextToken();
	assert(this->type != TokenType::EndOfFile);
}