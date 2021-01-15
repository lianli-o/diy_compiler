#include "parser.h"
#include <string>
#include <iostream>
#include <sstream>
#include <exception>

using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::getline;
using std::istringstream;
using std::runtime_error;

/*
path_prefix Ϊ·��ǰ׺��������������Ҫ���ļ����ڸ�·����
grammer.txt Ϊ�ķ�
*/

int main(int argc, char* argv[])
{	
	string path_prefix = "E:\\Workspace\\diy_compiler\\compiler\\Debug\\";
	Parser parser(path_prefix);

	while (true)
	{
		try {
			string line;

			cout << ">>> ";
			getline(cin, line);
			if (line.empty())
			{
				continue;
			}

			// ����Դ�ļ��������ʽ��>>> lianli code.txt -v
			// -v(erbose) ��ʾ����ģʽ�������Լ��Ϣ 
			istringstream is(line);
			string cmd;
			is >> cmd;
			if (cmd == "lianli")
			{
				string file_name, opt;
				bool verbose = false;
				is >> file_name;
				if (!is.eof())
				{
					is >> opt;
					if (opt == "-v")
					{
						verbose = true;
					}
				}
				
				parser.parser_analyze(file_name, verbose);
			}
			else {
				cout << "Cmd not found!\nCmd format: lianli code.txt" << endl;
			}
		}
		catch (runtime_error err)
		{
			cout << "************************************" << endl;
			cout << "ERROR! " << endl << err.what() << endl;
			cout << "************************************" << endl;
		}
	}
	
	return 0;
}


