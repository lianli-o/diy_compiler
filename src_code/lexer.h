#pragma once

#include "token.h"
#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>

// �ʷ�������
class Lexer {
public:
	Lexer(std::string name = ".\\code.txt");
	~Lexer();

	std::vector<Token*> tokens;		// ���ڱ���ʷ���������Ĵʷ���Ԫ

	Token* gen_token();
	int line = 1;				// ����
	bool fail() { return in.eof(); }	// �Ƿ�����ļ�β
	void bind_new_src_file(std::string name = ".\\code.txt"); // �����µ�Դ����
	void close_file() {
		if (in.is_open())
		{
			in.close();
		}
	}
private:
	char peek = ' ';			// Ԥ����һ���ַ�
	std::unordered_map<std::string, int> ID_table;	// �ַ�����
	std::ifstream in;			// Դ�ļ�

	// ���ܺ���
	Token* scan();
	Token* skip_blank(void);		// �����հ�
	Token* handle_slash(void);		// ����б�� /������ʹע�ͻ����
	Token* handle_relation_opterator(void);	// ���� > < = !�������ǹ�ϵ�������Ҳ�����Ǹ�ֵ�������ȡ�������
	Token* handle_digit(void);		// ��������
	Token* handle_word(void);		// ������ (��ʶ�� �� �ؼ���) �����»��߻���ĸ��ͷ��������»���/����/��ĸ
	Token* handle_char(void);		// �����ַ�����
	Token* handle_string(void);		// �����ַ�������
};


