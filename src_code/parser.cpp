#include "parser.h"
#include "string"
#include <iostream>
#include <vector>
#include <exception>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::runtime_error;

// ���ʷ���Ԫ Token תΪ�﷨�����е��ս��
// ��Ŀǰ������ķ����� id, num �Ǵ����Եģ���Щ����Ҫ����Ӧ���Դ��ݸ������������
Parser::sym_t Parser::token_to_grammer_sym(Token* token)
{	
	Parser::sym_t grammer_sym;
	
	if (token->tag < 255)	
	{
		// �ǵ������ţ�����ʷ���Ԫֱ�ӷ��ش��ؼ���
		// { } ; , = + - * / ! ( ) < >
		grammer_sym.push_back(static_cast<char>(token->tag));
	}
	else if (token->tag > Token::KEYWORD_POS)
	{
		// �ǹؼ��֣�����ʷ���Ԫֱ�ӷ��ش��ؼ���
		// int real if then else while do or and true false
		grammer_sym = dynamic_cast<Word*>(token)->lexeme;
	}
	else if (token->tag == Token::REL_OPT)
	{
		// �ǹ�ϵ�����������ʷ���Ԫֱ�ӷ��ش��ؼ���
		// == !=  <= >= 
		grammer_sym = dynamic_cast<RelOpt*>(token)->lexeme;
	}
	else if (token->tag == Token::NUM)
	{
		// num ���ֳ���
		grammer_sym = "num";
	}
	else if (token->tag == Token::ID)
	{
		// id ��ʶ��
		grammer_sym = "id";
	}

	return grammer_sym;
}

void Parser::parser_analyze(std::string new_file_name, bool verbose)
{
	Token* ptoken;
	vector<Parser::sym_t> grammer_program;	// ���ʷ��������ת��Ϊ�﷨����������ĸ�ʽ
	
	lexer.bind_new_src_file(path_prefix + new_file_name);

	// �ʷ����������������� lexer.tokens ��
	while (ptoken = lexer.gen_token())
	{
		ptoken->print(cout) << endl;
		grammer_program.push_back(token_to_grammer_sym(ptoken));
	}
	grammer_program.push_back("#");	// ����һ����������

	lr1.driver(grammer_program, lexer.tokens, verbose);

	cout << endl;
}


