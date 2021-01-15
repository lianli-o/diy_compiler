#include "lexer.h"
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cmath>
#include <unordered_map>

using std::unordered_map;
using std::runtime_error;
using std::ifstream;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::to_string;

// ����Ĺؼ�����Ŀ (����) Ҫ�� token.h �е� Token::Tags (��������) ���Ӧ (˳��Ҳ����һ��)
static const vector<string> Keywords{
	"true",
	"false",
	"if",
	"else",
	"then",
	"while",
	"do",
	"int",
	"real",
	"or",
	"and",
};

Lexer::Lexer(std::string name) : in(name)
{
	// �����йؼ��ִ����ַ�����
	int start = Token::TAGS::KEYWORD_POS + 1;	// �ؼ��ֿ�ʼ����ֵ
	for (auto it = Keywords.begin(); it != Keywords.end(); ++it)
	{
		ID_table[*it] = start + static_cast<int>(it - Keywords.begin());
	}
}

Lexer::~Lexer() {
	// ����ʹ�õĶ�̬���䣬�ǵ����Ҫ�ͷſռ�
	for (auto& token : tokens)
	{
		delete token;
	}
}

Token* Lexer::skip_blank(void)
{
	while (in && (peek == ' ' || peek == '\t' || peek == '\n'))
	{
		if (peek == '\n')
		{
			++line;
		}
		in >> peek;
	}
	in.unget();
	return nullptr;
}

Token* Lexer::handle_slash(void)
{
	char next_peek = in.peek();	
	if (in && next_peek == '/')	// ����ע��
	{
		in >> peek;		// �� / �����ж���
		while (in >> peek && peek != '\n')
		{
		}
		++line;		// ��ʱ peek Ϊ���з� �� Դ�������
	}
	else if (in && next_peek == '*')	// ����ע��
	{
		char prev_peek = ' ';
		in >> peek;		// �� * �����ж���
		while (in >> peek 
			&& !(prev_peek == '*' && peek == '/'))
		{
			prev_peek = peek;
			if (peek == '\n')
			{
				++line;
			}
		}
	}
	else {		// ����
		return	new Token('/');	
	}
	return nullptr;
}

Token* Lexer::handle_relation_opterator(void)
{
	char next_peek = in.peek();
	if (in && next_peek == '=')	// >= <= == !=
	{
		string lexeme;
		lexeme.push_back(peek);
		in >> next_peek;
		lexeme.push_back(next_peek);
		return new RelOpt(lexeme);
	}
	else {
		// = < > ! Tag��ֱ�ӵ�����ASCII����
		return new Token(peek);
	}
}
// ֧�ֿ�ѧ������
Token* Lexer::handle_digit(void)
{
	char next_peek = in.peek();
	auto to_digit = [](const char c) -> int { return (c - '0'); };
	double num = to_digit(peek);	// ���ֳ�����ֵ
	double decimal = 0;					// С������
	int decimal_bit = 0;				// С����λ��

	// �� e ֮ǰ�����ֶ��������浽 string �Ȼ���� stod(s) ת�� double ������Щ�����ǲ������
	while (in && isdigit(next_peek))
	{
		in >> peek;
		next_peek = in.peek();
		num = num * 10 + to_digit(peek);
	}
	if (in && next_peek == '.')
	{
		in >> peek;		// ����С����
		next_peek = in.peek();

		if (!in || !isdigit(next_peek))	// С��������Ҫ��һλ���֣���Ȼ����ƴд����
		{
			string line_msg = "Line" + to_string(line) + ": ";
			throw runtime_error(line_msg + "No number after \'.\' !");
		}

		in >> peek;		// ����С������һλ��
		next_peek = in.peek();
		decimal = to_digit(peek);
		decimal_bit = 1;
		
		while (in && isdigit(next_peek))
		{
			in >> peek;
			next_peek = in.peek();
			decimal = decimal * 10 + to_digit(peek);
			++decimal_bit;
		}
		decimal /= pow(10, decimal_bit);
		num += decimal;
	}
	if (in && next_peek == 'e')
	{
		int exp = 0;		// ָ��
		int flag = 1;		// ָ������
		in >> peek;			// ���� e
		next_peek = in.peek();
		if (in && (next_peek == '+' || next_peek == '-'))
		{
			in >> peek;
			flag = (peek == '+') ? 1 : -1;
			next_peek = in.peek();
		}
		if (!isdigit(next_peek))	// e �����Ҫ��һλ���֣���Ȼ����ƴд����
		{
			string line_msg = "Line" + to_string(line) + ": ";
			throw runtime_error(line_msg + "No number after \'e\' !");
		}
		while (isdigit(next_peek) && in >> peek)
		{
			next_peek = in.peek();
			exp = exp * 10 + to_digit(peek);
		}
		num = num * pow(10, exp);
	}

	return new Num(num);
}

Token* Lexer::handle_word(void)
{
	string lexeme;
	char next_peek;
	Token* token;

	lexeme.push_back(peek);
	next_peek = in.peek();

	while (in && (isalpha(next_peek) || isdigit(next_peek) || next_peek == '_'))
	{
		in >> peek;
		
		lexeme.push_back(peek);
		next_peek = in.peek();
	}

	auto it = ID_table.find(lexeme);
	if (it == ID_table.end())	// �ô��ز��ڹؼ��ֱ���
	{
		token = new Word(lexeme, Token::ID);
	}else{
		token = new Word(lexeme, it->second);
	}

	return token;
}

Token* Lexer::handle_char(void)
{
	Token* token = nullptr;
	if (in)
	{
		in >> peek;
		char next_peek = in.peek();
		if (next_peek != '\'')
		{
			string line_msg = "Line" + to_string(line) + ": ";
			throw runtime_error(line_msg + "Unmatched \'!");
		}
		token = new Char(peek);
		in >> peek;	// �����ұߵ� '
	}
	return token;
}

Token* Lexer::handle_string(void)
{
	string tmp;
	while(in >> peek && peek != '\"')
	{
		tmp.push_back(peek);
		if (peek == '\n')
		{
			++line;
		}
	}
	if (!in && peek != '\"')
	{
		string line_msg = "Line" + to_string(line) + ": ";
		throw runtime_error(line_msg + "Unmatched \"!");
	}
	return new String(tmp);
}

Token* Lexer::scan(){
	Token *token;	// ʶ����Ĵʷ���Ԫ
	if (!(in >> peek))
	{
		return nullptr;
	}
	char judger = peek;			// ���� switch case ���������ж�

	if (isdigit(judger))
	{
		judger = '0';			// �������ֶ�ת�� 0
	}
	else if (isalpha(judger))
	{
		judger = 'a';			// ������ĸ��ת�� a
	}

	switch (judger)
	{
	case ' ': case '\t': case '\n':
		token = skip_blank();		// �����հ�
		break;
	case '/':
		token = handle_slash();	// ����ע�ͺͳ���
		break;
	case '>': case '<': case '!': case '=':
		token = handle_relation_opterator();
		break;
	case '0':					// ��������
		token = handle_digit();
		break;
	case 'a': case '_':			// ���� ��ʶ�� / �ؼ��� 
		token = handle_word();
		break;
	case '\'':		// �����ַ�����
		token = handle_char();
		break;
	case '\"':		// �����ַ�������
		token = handle_string();
		break;
	default:	// �����������ַ�����ɴʷ���Ԫ
		token = new Token(peek);
		break;
	}
	if (token)
	{
		token->line = line;
	}
	
	return token;
}

Token* Lexer::gen_token()
{
	Token* t = nullptr;

	while (in && (t = this->scan()) == nullptr)
	{
	}
	// ���� �� ����һ���ʷ���Ԫ
	if (t)
	{
		tokens.push_back(t);
		return t;
	}
	return nullptr;
}

void Lexer::bind_new_src_file(string name)
{
	close_file();
	in.open(name);

	if (!in)
	{
		throw runtime_error("Wrong file name!\n");
	}
	in >> std::noskipws;	// ����ʱ������Դ�����еĿհף���ΪҪ��������Լ�������

	line = 1;
	peek = ' ';
	// ����ʹ�õĶ�̬���䣬�ǵ����Ҫ�ͷſռ�
	// ����ʹ�õĶ�̬���䣬�ǵ����Ҫ�ͷſռ�
	for (auto& token : tokens)
	{
		delete token;
	}
	tokens = vector<Token*>();

	// �����йؼ��ִ����ַ�����
	ID_table = unordered_map<string, int>();
	// �����йؼ��ִ����ַ�����
	int start = Token::TAGS::KEYWORD_POS + 1;	// �ؼ��ֿ�ʼ����ֵ
	for (auto it = Keywords.begin(); it != Keywords.end(); ++it)
	{
		ID_table[*it] = start + static_cast<int>(it - Keywords.begin());
	}
}
