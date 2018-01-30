#pragma once

#include <stdint.h>
#include <string>

#define ARG_NULL 0
#define ARG_ADDR 1
#define ARG_NUM  2
#define ARG_IO   3

struct OpCode
{
	uint16_t value;
	uint16_t mask;
	int argtype;
	std::string symbol;
};

// "null-terminated" array
extern OpCode opcodes[];