#pragma once

#include <string>
#include <iostream>

// �ʷ���Ԫ
class Token {
public:
	// �����Ǵʷ���Ԫ�Ĵ������
	// �ô��� char (255) ����ֵ��ʾ�ս����
	// С�� 255 ������  operator (�����),  delimiter (���)
	enum TAGS : unsigned {
		NUM = 256,		// ��ֵ����
		ID,				// ��ʶ��
		REL_OPT,		// ��ϵ�����
		CHAR,			// �ַ�����
		STRING,			// �ַ�������
		// ����Ϊ�ؼ��֣����ô�д��ʾ
		// ***********��������ӹؼ���֮��ҲҪ�� lexer.cpp �� Keywords �������Ӧ��Ŀ***********
		KEYWORD_POS,	// ��������ǹؼ��ֿ�ʼ����ֵ
		TRUE,
		FALSE,
		IF,
		ELSE,
		THEN,
		WHILE,
		DO,
		INT,
		REAL,
		OR,
		AND,
	};

	Token(const int &t = 0) : tag(t) {}
	virtual ~Token() = default;
	virtual std::ostream& print(std::ostream& os) { os << "line" << line << ": <" << static_cast<char>(tag) << '>'; return os; }

	int tag;		// �������
	size_t line;	// ÿ���ʷ���Ԫ���ֵ�����
};

// ��ֵ����
// ֧�ֿ�ѧ������
class Num : public Token {
public:
	Num() = default;
	Num(const double& v): Token(TAGS::NUM), value(v){}
	std::ostream& print(std::ostream& os) override { 
		os << "line" << line << ": <" << "NUM" << ',' << value << '>';
		return os;
	}
	std::string type;	// int / real Ŀǰû�ã����г�����ֵ����������������
	const double value;
};

// ��ʶ�� & �ؼ���
class Word : public Token {
public:
	Word() = default;
	Word(const std::string& v, int t = TAGS::ID) : Token(t), lexeme(v) {}
	std::ostream& print(std::ostream& os) override {
		std::string t = (tag == TAGS::ID) ? "ID" : "KEY_WORD";
		os << "line" << line << ": <" << t << ',' << lexeme << '>';
		return os;
	}

	std::string lexeme;	// ����
};

// ��ϵ����� >= <= != ==
// > < �����ǵ������ţ�Tag ��ʱ��ֱ����Ϊ���ǵ� Ascii ����
class RelOpt : public Token {
public:
	RelOpt() = default;
	RelOpt(const std::string& v, int t = TAGS::REL_OPT) : Token(t), lexeme(v) {}
	std::ostream& print(std::ostream& os) override {
		os << "line" << line << ": <" << "RELOPT" << ',' << lexeme << '>';
		return os;
	}

	const std::string lexeme;	// ����
};

// �ַ�����
class Char : public Token {
public:
	Char() = default;
	Char(const char& v) : Token(CHAR), value(v) {}
	std::ostream& print(std::ostream& os) override {
		os << "line" << line << ": <\'" << value << "\'>";
		return os;
	}

	char value;	// �ַ�����ֵ
};

// �ַ�������
class String : public Token {
public:
	String() = default;
	String(const std::string& v) : Token(STRING), value(v) {}
	std::ostream& print(std::ostream& os) override {
		os << "line" << line << ": <\"" << value << "\">";
		return os;
	}

	std::string value;	// �ַ�������ֵ
};


