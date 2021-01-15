#pragma once

#include "lexer.h"
#include "lr1.h"
#include <string>
#include <unordered_set>
#include <memory>


// �﷨������
class Parser {
public:
	using sym_t = Production::sym_t;

	Parser(std::string pathprefix = "./", std::string code_file_name = "code.txt")
		: lexer(pathprefix + code_file_name), lr1(pathprefix + "grammer.txt"), path_prefix(pathprefix){}

	void parser_analyze(std::string new_file_name = "", bool verbose = true);	// �﷨����
private:
	sym_t token_to_grammer_sym(Token* token);

	Lexer lexer;	// �ʷ�������
	LR1Processor lr1;	// LR1 ������
	std::string path_prefix;	// Դ�ļ�·��ǰ׺
};


