#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <map>
#include <vector>
#include <functional>

#include "opcodes.hpp"
#include "tokens.hpp"
#include "parser.hpp"

void disasm(FILE * dest, uint16_t * mem, size_t len, uint16_t offset = 0x0000)
{
	for(size_t i = 0; i < len; i++)
	{
		uint16_t cmd = mem[i];
		
		OpCode * code = nullptr;
		for(OpCode * op = opcodes; op->mask; op++)
		{
			if((cmd & op->mask) != op->value)
				continue;
			code = op;
		}
		
		fprintf(dest, "%04X:\t%04X", offset + i, cmd);
		if(code)
		{
			fprintf(dest, "\t%s", code->symbol);
			switch(code->argtype)
			{
				case ARG_NULL:
					break;
				case ARG_ADDR:
					assert(code->mask == 0xF000);
					if(cmd & 0x0800)
						fprintf(dest, " I");
					fprintf(dest, "\t$%X", 0x07FF & (cmd & (~code->mask)));
					break;
				case ARG_NUM:
					fprintf(dest, "\t%d", (cmd & (~code->mask)));
					break;
				case ARG_IO:
					assert(code->mask == 0xFF00);
					fprintf(dest, "\t%d, %d", (cmd & 0xF0) >> 4, (cmd & 0x0F));
					break;
			}
		}
		fprintf(dest, "\n");
		
	}
}

static void disasm_test()
{
	uint16_t example[] = 
	{
		0x1081, // JMP 81    "Einstiegspunkt nach erfolgreichem Laden"
		0x03E9, // DOT E,9   "Wähle Gerät
		0x02E0, // DIN E,0   "Lese Status
		0x0AC1, // SRC L 1   "Schiebe 'Daten empfangen'-Bit nach Bit 0
		0x0820, // SAM       "Prüfe auf Bit 0
		0x1081, // JMP 81    "Loop
		0x02ED, // DIN E,D   "Lese Byte
		0x3800, // STB 800   "Schreibe Byte nach Addr(Index)
		0x0501, // DXS 1     "Für erstes Byte: setze Index auf
		0x0402, // IXS 2     "erstes Byte, sonst: Index++
		0x0130, // CAX       "--
		0x107F, // JMP 7F    "Springe zu Einstiegspunkt
	};

	disasm(stdout, example, 0xC, 0x7F);
}

struct asmword_t
{
	uint16_t value;
	bool allocated;
	
	asmword_t() : value(0xFFFF), allocated(false) { }
	asmword_t(uint16_t value) : value(value), allocated(true) { }
	
	operator uint16_t() const { return this->value; }
};

OpCode const * findOpcode(std::string const & sym)
{
	OpCode * code = nullptr;
	for(OpCode const * op = opcodes; op->mask; op++)
	{
		if(sym != op->symbol)
			continue;
		return op;
	}
	return nullptr;
}

class Assembler
{
	typedef std::function<void()> patch_t;
	static constexpr int memorySize = 32768;
	int cursor;
	asmword_t memory[memorySize];
	std::map<std::string, int> labels;
	std::vector<patch_t> patches;

	/**
	 * @returns -1 on error, -2 on "requires patch", else integer value
	 */
	int toInt(Token const & tok, bool label = false)
	{
		switch(tok.type)
		{
			case TokenType::Number:
				return std::stoi(tok.text);
			case TokenType::HexNumber:
				return std::stoi(tok.text.substr(1), nullptr, 16);
			case TokenType::Name:
				if(label)
				{
					auto it = labels.find(tok.text);
					if(it != labels.end())
					{
						return (*it).second;
					}
					else
					{
						// return "install patch"
						// fprintf(stderr, "%s must be patched in line %d\n", tok.text.c_str(), tok.line);
						return -2;
					}
				}
				// FALLTHROUGH
			default:
				if(label)
					fprintf(stderr, "%s is neither a number nor label in line %d\n", tok.text.c_str(), tok.line);
				else
					fprintf(stderr, "%s is not a number in line %d\n", tok.text.c_str(), tok.line);
				return -1;
		}
	}
	
public:
	void assemble()
	{
		cursor = 0;
		while(true)
		{
			Token tok = getNextToken();
			if(tok.type == TokenType::EndOfFile)
				break;
			if(tok.type == TokenType::LineBreak)
				continue;
			switch(tok.type)
			{
				case TokenType::Directive: {
					assert(tok.text == ".ORG");
					tok.next();
					int addr = toInt(tok);
					if(addr == -1) {
						continue;
					}
					this->cursor = addr;
					break;
				}
				case TokenType::Label:
					labels.emplace(tok.text.substr(0, tok.text.length() - 1), cursor);
					break;
				case TokenType::Name: {
					OpCode const * op = findOpcode(tok.text);
					if(op == nullptr) {
						fprintf(stderr, "unknown opcode in line %d: %s\n", tok.line, tok.text.c_str());
						break;
					}
					uint16_t value = op->value;
					switch(op->argtype)
					{
						case ARG_NULL:
							break;
						case ARG_ADDR: {
							tok.next();
							if(tok.text == "I") {
								value |= 0x0800;
								tok.next();
							}
							int val = toInt(tok, true);
							if(val == -1)
								continue;
							if(val == -2) {
								fprintf(stderr, "patch for line %d: %s\n", tok.line, tok.text.c_str());
								int pos = cursor;
								patches.emplace_back([this,pos,value,op,tok]() {
									fprintf(stderr, "patching line %d\n", tok.line);
									int val = toInt(tok, true);
									assert(val >= 0);
									memory[pos] = value | ((~op->mask) & val);
								});
								continue;
							}
							value |= (~op->mask) & val;
							break;
						}
						case ARG_NUM: {
							tok.next();
							int val = toInt(tok);
							if(val == -1)
								continue;
							value |= (~op->mask) & val;
							break;
						}
						case ARG_IO: {
							tok.next();
							int dev = toInt(tok);
							if(dev == -1)
								continue;
							tok.next();
							if(tok.type != TokenType::Comma) {
								fprintf(stderr, "syntax error in line %d: comma expected\n", tok.line);
								continue;
							}
							tok.next();
							int fun = toInt(tok);
							if(dev == -1)
								continue;
							value |= ((0x00F0) & (dev << 4));
							value |= ((0x000F) & fun);
							break;
						}
						default:
							assert(false); 
					}
					memory[cursor] = value;
					cursor++;
					
					tok.next();
					if(tok.type != TokenType::LineBreak) {
						fprintf(stderr, "syntax error in line %d: comma expected\n", tok.line);
					}
					
					break;
				}
				default:
					fprintf(stderr, "syntax error in line %d: %s\n", tok.line, tok.text.c_str());
					break;
			}
		}
		
		// install patches:
		for(patch_t const & patch : patches)
			patch();
	}
	
	void dump()
	{
		for(int i = 0; i < memorySize; i++)
		{
			if(!memory[i].allocated)
				continue;
			if(i > 0 && !memory[i - 1].allocated)
				printf("\n");
			printf("%04X: %04X\n", i, memory[i].value);
		}
	}
};

static void asm_test()
{
	Assembler as;
	as.assemble();
	as.dump();
}

int main(int argc, char ** argv)
{
	asm_test();
	return 0;
}