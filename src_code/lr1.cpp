#include "lr1.h"
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <stack>
#include <queue>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <unordered_map>

using std::runtime_error;
using std::ostringstream;
using std::istringstream;
using std::endl;
using std::cout;
using std::string;
using std::unordered_set;
using std::vector;
using std::find;
using std::stack;
using std::queue;
using std::setw;
using std::getline;
using std::make_pair;
using std::unordered_map;
using std::ostream;

const Production::sym_t Production::epsilon = string("$");	// �չ���
const Grammer::sym_t Grammer::end_of_sentence = string("#");		// ���ӽ�β

// �����Ԫʽ
std::ostream& operator<< (std::ostream& os, const Quad& q)
{
	os << "( " << q.opt << ", " << q.lhs << ", " << q.rhs << ", " << q.dest << " )";
	return os;
}

// �������ʽ�ĸ�ʽ��Stmts -> Stmts Stmt
// ��һ�е����� # ��Ϊ����ʽ��������ı�־
bool Production::input_production(const std::string& p)
{
	if (p[0] == '#')	// ����ʽ���ֽ���
	{
		return false;
	}

	istringstream in(p);

	in >> left;
	string _;
	in >> _;	// ���� ->

	while (in >> _)
	{
		right.push_back(_);
	}

	return true;
}

std::ostream& operator<<(std::ostream& out, Production& p)
{
	out << p.left << " -> ";
	for (const auto& r : p.right)
	{
		out << r << " ";
	}

	return out;
}

std::istream& operator>>(std::istream& in, Grammer& g)
{
	Production tmp;
	Grammer::sym_t sym;
	queue<Grammer::sym_t> q;
	string line;

	// ���������ķ�
	// �������ʽ�м��п���
	while (getline(in, line))
	{
		if (line.size() == 0)
		{
			continue;
		}
		if (tmp.input_production(line))
		{
			g.productions.push_back(tmp);
			tmp = Production();
		}
		else {
			break;
		}
	}
	// ������ս��
	while ((in >> sym) && sym != "#")
	{
		g.N.insert(sym);
		q.push(sym);
	}

	// �����ս��
	while ((in >> sym) && sym != "#")
	{
		g.T.insert(sym);
		g.TN.push_back(sym);
	}
	g.TN.push_back(Grammer::end_of_sentence);	// �� # ����TN��

	// �����ս������TN��������TN����˳��������ս������ս������˳��������˳��һ�£���������֮���ӡLR1������
	while (!q.empty())
	{
		g.TN.push_back(q.front());
		q.pop();
	}

	g.first_set();	// �� FIRST ��

	return in;
}

std::ostream& operator<<(std::ostream& out, Grammer& g)
{
	out << "�ս��: \n";
	int cnt = 0;
	for (auto it = g.T.begin(); it != g.T.end(); ++it, ++cnt)
	{
		out << *it << " ";
		if (cnt % 10 == 0 && cnt != 0)	// һ��ʮ�����Ž������
		{
			out << "\n";
		}
	}
	out << "\n���ս��: \n";
	cnt = 0;
	for (auto it = g.N.begin(); it != g.N.end(); ++it, ++cnt)
	{
		out << *it << " ";
		if (cnt % 10 == 0 && cnt != 0)	// һ��ʮ�����Ž������
		{
			out << "\n";
		}
	}
	out << "\n����ʽ: \n";
	for (size_t i = 0; i != g.productions.size(); ++i)
	{
		out << g.productions[i] << endl;
	}

	return out;
}

// �����õ� FIRST ��
std::ostream& Grammer::output_first_set(std::ostream& os) const
{
	os << "FIRST SET:\n";
	for (auto it = first.begin(); it != first.end(); ++it)
	{
		os << it->first << ":  ";
		const auto& fs = it->second;
		for (auto it1 = fs.begin(); it1 != fs.end(); ++it1)
		{
			os << *it1 << "  ";
		}
		os << "\n";
	}

	return os;
}

void Grammer::first_set()
{
	// �ս���� FIRST ��Ϊ������
	for (auto it = T.begin(); it != T.end(); ++it)
	{
		first[*it].insert(*it);
	}

	// �����ս���� FIRST �������仯ʱѭ��
	bool change = true;
	while (change)
	{
		change = false;
		// ö��ÿ������ʽ
		for (auto it = productions.begin(); it != productions.end(); ++it) {
			sym_t left = it->get_left();	// ����ʽ��
			vector<sym_t> right = it->get_right();	// ����ʽ�Ҳ�
			// A -> $
			// ����Ҳ���һ�������ǿջ������ս��������뵽�󲿵� FIRST ���У����Ӵ�����Ĳ���ʽ��ɾ���ù���
			if (first_is_T(*it))
			{
				// ���� FIRST ���Ƿ��Ѿ����ڸ÷���
				if (first[left].find(right[0]) == first[left].end())
				{
					// FIRST �������ڸ÷���
					change = true;	// ��ע FIRST �������仯��ѭ������
					first[it->get_left()].insert((it->get_right())[0]);
				}
			}
			else {	// ��ǰ�����Ƿ��ս��
				bool next = true;	// �����ǰ���ſ����Ƴ��գ������ж���һ������ 
				size_t idx = 0;		// ���жϷ��ŵ��±�

				while (next && idx != right.size()) {
					next = false;
					sym_t n = right[idx];	// ��ǰ������Ҳ����ս��

					for (auto it = first[n].begin(); it != first[n].end(); ++it) {
						// �ѵ�ǰ���ŵ� FIRST ���зǿ�Ԫ�ؼ��뵽�󲿷��ŵ� FIRST ����
						if (*it != Production::epsilon
							&& first[left].find(*it) == first[left].end()) {
							change = true;
							first[left].insert(*it);
						}
					}
					// ��ǰ���ŵ� FIRST �����п�, ��� next Ϊ�棬idx �±�+1
					if (first[n].find(Production::epsilon) != first[n].end()) {
						if (idx + 1 == right.size()
							&& first[left].find(Production::epsilon) == first[left].end())
						{
							// ��ʱ˵������ʽ�󲿿����Ƴ��գ������Ҫ�� epsilon ���� FIRST ��
							change = true;
							first[left].insert(Production::epsilon);
						}
						else {
							next = true;
							++idx;
						}
					}
				}
			}
		}
	}
}

std::unordered_set<Grammer::sym_t> Grammer::first_set_string(const std::vector<sym_t>& s) const
{
	unordered_set<sym_t> res;

	for (size_t i = 0; i != s.size(); ++i)
	{
		if (first_is_T(s[i]))	// �����ս�� / �չ��򣬼��� FIRST ��
		{
			res.insert(s[i]);
			break;
		}
		else {
			// �������ս������Ҫ�Ѹ÷��ս���� FIRST ���г��˿չ���֮��ķ��Ŷ����� res
			for (const auto& f : first.at(s[i]))
			{
				if (f != Production::epsilon)
				{
					res.insert(f);
				}
			}
			// �Ʋ����չ���ֱ�ӽ���
			if (first.at(s[i]).find(Production::epsilon) == first.at(s[i]).end())
			{
				break;
			}
			// �÷��ս����������Ƴ��չ�����Ҫ�鿴��һ������
			else if (i + 1 == s.size())	// �����һ�����ս�����ҿ����Ƴ��չ�����չ���ҲҪ���� res
			{
				res.insert(Production::epsilon);
			}
		}
	}
	return res;
}

// �Ƚ�����LR1��Ŀ�Ƿ����
bool operator==(const LR1Item& item1, const LR1Item& item2)
{
	return (item1.next == item2.next) && (item1.loc == item2.loc)
		&& (item1.get_left() == item2.get_left()) && (item1.get_right() == item2.get_right());
}

LR1DFA::LR1_items_t LR1DFA::closure(const LR1_items_t& items)
{
	LR1_items_t closure_set;	// �����õıհ�
	stack<LR1Item> s;

	// ��ԭ��е�ÿ����Ȱ����Ǵ浽��ջ��
	for (const auto& item : items)
	{
		s.push(item);
	}
	// ÿ�δӶ�ջ�ﵯ��һ������������ӵ����� LR1 ��ѹ��ջ��
	while (!s.empty())
	{
		LR1Item item = s.top();
		s.pop();
		closure_set.insert(item);

		vector<sym_t> right = item.get_right();	// ����ʽ�Ҳ�
		if (item.loc == right.size())	// ��Լ�ֱ������
		{
			continue;
		}
		// [A -> alpha . B beta, u], B -> gamma   =>   [B -> .gamma, b], b is in FIRST(beta u)
		sym_t B = right[item.loc];
		if (item.loc != right.size() && grammer.N.find(B) != grammer.N.end())
		{
			vector<sym_t> beta_u;
			if (item.loc + 1 != right.size())	// beta ��Ϊ��
			{
				beta_u.push_back(right[item.loc + 1]);
			}
			beta_u.push_back(item.next);

			unordered_set<sym_t> first_b_u = grammer.first_set_string(beta_u);	// FIRST(beta u)

			// ����ÿ�� B -> gamma ����ʽ���� [B -> .gamma, b] ��ջ
			for (const auto p : grammer.productions)
			{
				if (p.get_left() == B)
				{
					vector<sym_t> r = p.get_right();
					// ��Ϊ֮ǰ�����ʱ�򶼰ѿչ���$ֱ�ӵ����ս�������������������������
					if (r.size() == 1 && r[0] == Production::epsilon)
					{
						r = vector<sym_t>{};
					}

					for (const auto& b : first_b_u)
					{
						LR1Item tmp = LR1Item(B, r, b, 0);
						if (closure_set.find(tmp) == closure_set.end())
						{	// ֻ����û�мӽ��հ�����Ŀ��
							s.push(tmp);
						}
					}
				}
			}
		}
	}

	return closure_set;
}

LR1DFA::LR1_items_t LR1DFA::go_to(const LR1_items_t& items, sym_t sym)
{
	LR1_items_t res;
	for (const auto& item : items)
	{
		auto right = item.get_right();
		if (item.loc != right.size() && right[item.loc] == sym)
		{
			res.insert(LR1Item(item.get_left(), right, item.next, item.loc + 1));
		}
	}
	return closure(res);
}

// ��ӡ LR1 ��Ŀ��
std::ostream& LR1DFA::print_lr1_items(const LR1_items_t& items, std::ostream& os) const
{
	for (const auto item : items)
	{
		vector<sym_t> right = item.get_right();
		os << "[ " << item.get_left() << " -> ";
		for (size_t i = 0; i != right.size(); ++i)
		{
			if (i == item.loc)
			{
				os << ". ";
			}
			os << right[i] << " ";
		}
		if (item.loc == right.size())	// ���ڲ���ʽ����Ǹ���Լ��
		{
			os << ".";
		}
		os << "," << item.next << " ]     ";
	}

	return os;
}

/*
* ����Ҫ��
* ����Ϊ�����ķ����ҵ�һ������ʽ��Ϊ��ʼ����
* ����Ϊ�������ķ���������ζ��LR�������г�ͻ����˱����ڳ������ֶ������ͻ
* Ŀǰֻ֧�� if else ���Ķ�����
*
* �����ʽ��
* ����ʽ...
* #
* �ս�� #
* ���ս�� #
*
* ����������(���������֮��һ��Ҫ�пո��ս���ͷ��ս��֧�ֶ����ĸ)
* A -> S
* S -> C C
* C -> c C
* C -> d
* #
* A S C #
* c d #
*/
LR1Processor::LR1Processor(std::string file_name) : in(file_name) {
	in >> grammer;
	dfa.grammer = grammer;

	cout << "�����ķ�: \n" << grammer << endl;	// ����ķ�
	grammer.output_first_set(cout) << endl;		// �����õ�FIRST��
	construct_table();			// ����LR1������

	if (!ambiguity_terms.empty())
	{
		solve_ambiguity();	// �������������
		if (!ambiguity_terms.empty())
		{
			// ���������⻹û�н��
			cout << endl << "************************************" << endl;
			cout << "Warning: This is not an LR1 grammer!" << endl;
			cout << "The ambiguity cannot be solved!" << endl;
			cout << "************************************" << endl << endl;
		}
		else {
			cout << endl << "************************************" << endl;
			cout << "Ambiguity solved!" << endl;
			cout << "���������֮��� LR1 ������: " << endl;
		}
	}

	print_lr1_table(cout) << endl << endl;		// ��ӡLR1������
}

std::ostream& operator<<(std::ostream& out, LR1Processor& l)
{
	out << l.grammer;
	return out;
}

void LR1Processor::construct_table()
{
	queue<LR1DFA::LR1_items_t> q;	// ���������� LR1 �
	unordered_map<LR1DFA::LR1_items_t, size_t> items_set;	// ��¼�Ѿ�������� LR1 �������LR1�淶����е���ţ���ֹ�ظ�����

	// ��ʼ��Ŀ��[S' -> .S, #]
	LR1DFA::LR1_items_t lr1_items{ LR1Item(grammer.productions[0], Grammer::end_of_sentence, 0) };
	lr1_items = dfa.closure(lr1_items);	// closure([S' -> .S, #])
	q.push(lr1_items);	// ���������������

	lr1_sets.push_back(lr1_items);	// ����ʼ��Ŀ�������յ� LR1 �淶���
	action_table.push_back({});	// �Ƚ���Ӧ�Ŀձ������action��
	goto_table.push_back({});	// �Ƚ���Ӧ�Ŀձ������goto��
	items_set.insert(make_pair(lr1_items, lr1_sets.size() - 1));	// �+���

	size_t idx = 0;	// ��¼�������LR1�����������

	while (!q.empty())
	{
		lr1_items = q.front();
		q.pop();

		// �鿴������Ƿ��п��Թ�Լ����Ŀ���������Ӧ����ǰ�����Ž������� ACTION ��
		for (const auto& item : lr1_items)
		{
			if (item.loc == item.len())	// �ɹ�Լ
			{
				// �ҵ���ǰ��Լ�Ĺ����Ӧ�����
				size_t i = 0;
				for (i = 0; i != grammer.productions.size(); ++i)
				{
					sym_t grammer_left = grammer.productions[i].get_left();
					sym_t item_left = item.get_left();
					vector<sym_t> grammer_right = grammer.productions[i].get_right();
					vector<sym_t> item_right = item.get_right();

					if (grammer_left == item_left
						&& (grammer_right == item_right
							|| (item_right.size() == 0 && grammer_right.size() == 1 && grammer_right[0] == Production::epsilon)))// �չ���
					{
						break;
					}
				}
				if (i == 0)	// �õ�һ������ʽ��Լ��˵����Լ����ʼ����
				{
					action_table[idx][item.next] = string("ACC");
				}
				else {
					ostringstream formatted;
					formatted << "R" << i;
					action_table[idx][item.next] = formatted.str();
				}
			}
		}

		// ��ÿ�����ţ����� GOTO ���µ� LR1 �
		// ͬʱ�� action �� goto ��
		for (const auto& t : grammer.T)	// action ��
		{
			if (t != Production::epsilon && t != Grammer::end_of_sentence)
			{
				LR1DFA::LR1_items_t tmp = dfa.go_to(lr1_items, t);	// �µ� LR1 � �� �Ѿ����ֹ��� LR1 �
				if (tmp.size() != 0)	// ���Ч��˵�� t �ڸ�״̬�ǺϷ�����
				{
					auto it = items_set.find(tmp);
					ostringstream formatted;	// ������� ACTION ����ַ�����

					if (it != items_set.end())	// ����Ѿ����ֹ���
					{
						formatted << "S" << it->second;
					}
					else {	// �µ� LR1 �
						q.push(tmp);
						lr1_sets.push_back(tmp);	// �������յ� LR1 �淶���
						action_table.push_back({});	// �Ƚ���Ӧ�Ŀձ������action��
						goto_table.push_back({});	// �Ƚ���Ӧ�Ŀձ������goto��
						items_set.insert(make_pair(tmp, lr1_sets.size() - 1));

						formatted << "S" << lr1_sets.size() - 1;
					}
					// ��ת����ϵ���� ACTION ��
					if (action_table[idx].find(t) != action_table[idx].end())
					{
						// LR1 �������ͻ
						cout << endl << "************************************" << endl;
						cout << "Warning: This is not an LR1 grammer!" << endl;
						action_table[idx][t] += " " + formatted.str(); // ������������
						cout << "��ͻ�� [ " << idx << ", " << t << " ]: " << action_table[idx][t] << endl << endl;
						ambiguity_terms.push_back({ static_cast<int>(idx), t });
						cout << "************************************" << endl << endl;
					}
					else {
						action_table[idx][t] = formatted.str();
					}
				}
			}
		}
		for (const auto& n : grammer.N)	// goto ��
		{
			if (n != grammer.productions[0].get_left())	// ��ʼ���ŵĹ�Լ������ACTION���е�ACCʱ���Ѿ�����ˣ������GOTO����
			{
				LR1DFA::LR1_items_t tmp = dfa.go_to(lr1_items, n);	// �µ� LR1 � �� �Ѿ����ֹ��� LR1 �

				if (tmp.size() != 0)	// ���Ч��˵�� n �ڸ�״̬�ǺϷ�����
				{
					auto it = items_set.find(tmp);
					ostringstream formatted;	// ������� GOTO ����ַ�����

					if (it != items_set.end())	// ����Ѿ����ֹ���
					{
						formatted << it->second;
					}
					else {	// �µ� LR1 �
						q.push(tmp);
						lr1_sets.push_back(tmp);	// �������յ� LR1 �淶���
						action_table.push_back({});	// �Ƚ���Ӧ�Ŀձ������action��
						goto_table.push_back({});	// �Ƚ���Ӧ�Ŀձ������goto��
						items_set.insert(make_pair(tmp, lr1_sets.size() - 1));

						formatted << lr1_sets.size() - 1;
					}
					// ��ת����ϵ���� GOTO ��
					if (goto_table[idx].find(n) != goto_table[idx].end())
					{
						// LR1 �������ͻ
						cout << "Warning: This is not an LR1 grammer!" << endl << endl;
						goto_table[idx][n] = " " + formatted.str();	// ������������
						ambiguity_terms.push_back({ static_cast<int>(idx), n });
					}
					else {
						goto_table[idx][n] = formatted.str();
					}
				}
			}
		}
		++idx;
	}
}

std::ostream& LR1Processor::print_lr1_sets(std::ostream& os) const
{
	for (size_t i = 0; i != lr1_sets.size(); ++i)
	{
		os << "LR1Items " << i << ";\n";
		dfa.print_lr1_items(lr1_sets[i], os) << endl;
	}
	return os;
}

std::ostream& LR1Processor::print_lr1_table(std::ostream& os) const
{
	const long long col_width = 10;	// ���������һ�еĿ��
	const long long first_col_width = 10;
	const string err_str(" ");	// LR1�������д��������ô��ӡ�������ÿհף�Ҳ���Ի��ɡ�err��

	os << "LR1���: " << endl;
	print_lr1_sets(os) << endl; //��ӡ LR1 �淶���

	os << std::left;	// �����
	os << setw(first_col_width) << "STATE"
		<< setw((grammer.T.size() - 1) * col_width) << "ACTION"		// ȥ��$�Ŀ��
		<< setw(grammer.N.size() * col_width) << "GOTO" << endl;
	os << setw(first_col_width) << " ";

	for (const auto& c : grammer.TN)
	{
		os << setw(col_width) << c;
	}
	os << endl;

	for (size_t i = 0; i != lr1_sets.size(); ++i)
	{
		os << setw(first_col_width) << i;
		// ��ӡ action ���һ��
		for (size_t j = 0; j + 1 != grammer.T.size(); ++j)
		{
			auto it = action_table[i].find(grammer.TN[j]);
			if (it != action_table[i].end())
			{
				os << setw(col_width) << it->second;
			}
			else {
				os << setw(col_width) << err_str;
			}
		}
		// ��ӡ goto ���һ��
		for (size_t j = grammer.T.size() - 1; j != grammer.TN.size(); ++j)
		{
			auto it = goto_table[i].find(grammer.TN[j]);
			if (it != goto_table[i].end())
			{
				os << setw(col_width) << it->second;
			}
			else {
				os << setw(col_width) << err_str;
			}
		}
		os << endl;
	}
	os << std::right;	// �ָ��Ҷ���
	return os;
}

/*
�ú����д����� if ���Ķ��������⣬�������ƽ� - ��Լ��ͻ������ǰ������Ϊ else����ǿ���ƽ�
*/
void LR1Processor::solve_ambiguity_if()
{
	int change_flag = 1;
	while (change_flag)
	{
		change_flag = 0;
		for (auto it = ambiguity_terms.begin(); it != ambiguity_terms.end(); ++it)
		{
			if (it->second == "else")
			{
				istringstream is(action_table[it->first][it->second]);
				string tmp;
				while (is >> tmp)
				{
					if (tmp[0] == 'S')	// �ƽ���Ŀ
					{
						action_table[it->first][it->second] = tmp;	// ֻȡ�ƽ���Ŀ
						break;
					}
				}
				ambiguity_terms.erase(it);
				change_flag = 1;
				break;	// ȥ����һ����ͻ��֮�������������������һ�α�������Ϊɾ��Ԫ�غ��ʹ������ʧЧ
			}
		}
	}
}

/*
*
*���֮��������������������Ҫ�����ҲӦ���ڸú����������Ӧ�Ĺ��ܺ���
*/
void LR1Processor::solve_ambiguity()
{
	solve_ambiguity_if();
}


/* ���ܺ������ɵ�ǰ��״̬�Լ���һ��������ţ�ͨ����LR1�����������Ӧ�Ķ���
* �������Ϊ��Լ���� next Ϊ����ʽ���
* �������ΪGOTO���� next Ϊ��һ��״̬
*/
LR1Processor::TableAction_t LR1Processor::gen_next_mov(const int& state, sym_t next_sym, int& next) const
{
	// �����������ľ�Ϊ�Ϸ����ţ��Ƿ�����Ӧ���ڴʷ������׶α���
	if (grammer.T.find(next_sym) != grammer.T.end())
	{
		// ���ս������ ACTION ��
		auto it = action_table[state].find(next_sym);

		if (it == action_table[state].end())
		{
			// ACTION ���Ӧ��ĿΪ�գ��þ��Ӵ���
			return TableAction_t::ERR;
		}
		else {
			string action = it->second;
			if (action[0] == 'S')	// ����Ϊ�ƽ�
			{
				istringstream is(action);
				char _;
				is >> _ >> next;

				return TableAction_t::ACTION_READ;
			}
			else if (action[0] == 'A')	// ACC
			{
				return TableAction_t::ACC;
			}
			else {	// ����Ϊ��Լ
				istringstream is(action);
				char _;
				is >> _ >> next;

				return TableAction_t::ACTION_REDUCE;
			}
		}
	}
	else {
		// �Ƿ��ս������ GOTO ��
		auto it = goto_table[state].find(next_sym);

		if (it == goto_table[state].end())
		{
			// GOTO ���Ӧ��ĿΪ�գ��þ��Ӵ���
			return TableAction_t::ERR;
		}
		else {
			string go_to = it->second;
			istringstream is(go_to);

			is >> next;

			return TableAction_t::GOTO;
		}
	}
}

/*
* ������
* @state_stack: ״̬ջ
* @sym_stack: ����ջ
* @s: ���봮
* @p: ָʾ��ǰ�������봮���ĸ�λ��
* @os: ������Ϣ�������Ĭ��Ϊ std::cerr
*/
std::ostream& LR1Processor::err_proc(std::vector<sym_t>& s, size_t& p, const vector<Token*>& tokens, std::ostream& os)
{
	/*
	���м򵥵Ķ����δ���ָ���˼·���£�
		���ջ��״̬��Ӧ�� LR1 ��Ŀ������һ����Լ������ǰ������Ϊ ;  } )��
		��ǿ�ƽ��й�Լ�������Ӧ�ĳ�����Ϣ���������Ѷ�����Ӧ��  ;  } )

		��ˣ�����Դ�������ٸ������źͷֺŶ����ⲻ���� :)

		�Ժ���ʱ��Ļ��������Ӹ������ĳ��������~
	*/
	int now_state = state_stack.top();
	bool err_recover_flag = false;		// �Ƿ�ɹ����д���ָ�

	for (const auto& lr1_item : lr1_sets[now_state])
	{
		if (lr1_item.loc == lr1_item.len()
			&& (lr1_item.next == ";" || lr1_item.next == "}" || lr1_item.next == ")"))
		{
			size_t line = (p == 0) ? p : p - 1;	// ��Ϊ���øú�����ʱ���Ѿ�����ֽ�� ) } ; ��ǰһ�����ţ�����������к�Ӧ��Ϊǰһ���ַ����к�
			os << endl << "Line" << tokens[line]->line << ": " << endl;
			os << "Warning " << warning_num << ": Lack of " << lr1_item.next << endl;

			++warning_num;
			err_add_syms.push(lr1_item.next);	// ������ĳ�������ӽ��ʵ��ķֽ�������д���ָ�
			err_recover_flag = true;
			break;
		}
	}

	if (err_recover_flag == false)	// �޷�����ָ���ֱ�ӱ���������е��������
	{
		ostringstream err_msg;

		err_msg << "Line" << tokens[p]->line << ": " << endl;
		err_msg << "state " << now_state << " can't accept " << s[p] << " !" << endl;
		err_msg << "possible symbols are: " << endl;
		for (const auto& t : grammer.T)
		{
			int _;
			if (TableAction_t::ERR != gen_next_mov(state_stack.top(), t, _))
			{
				err_msg << t << " ";
			}
		}
		throw runtime_error(err_msg.str());
	}

	return os;
}

void LR1Processor::init_state()
{
	err_add_syms = queue<sym_t>();
	state_stack = stack<int>();
	sym_stack = stack<sym_t>();
	attr_stack = stack<Attribute>();
	sym_table = vector<SymTable>();
	nxq = 1;
	quads = { Quad() };
	tmp_idx = 0;
	warning_num = 0;
}

// LR1 ������������ / �ܿس��򣻷����� '#' ��β�ľ���
void LR1Processor::driver(std::vector<sym_t>& s, std::vector<Token*> tokens, bool verbose, std::ostream& os)
{
	size_t p = 0;				// ָ�����봮��ָ��
	bool acc_flag = 0;
	ostringstream warning_msg;

	if (verbose)
	{
		os << endl << "LR1 analysis for \"";
		for (const auto& str : s)
		{
			os << str << " ";
		}
		os << "\" :" << endl;
	}

	// ��ʼ��״̬
	init_state();
	state_stack.push(0);
	sym_stack.push(Grammer::end_of_sentence);
	attr_stack.push(Attribute());

	warning_msg << "***********************************" << endl;

	while ((p != s.size() || !err_add_syms.empty())
		&& acc_flag == false)
	{
		int state = state_stack.top();
		int next = 0;
		TableAction_t next_mov;
		ostringstream _;
		bool use_err_buff_flag = !err_add_syms.empty();	// ����ָ��н��ַ����뵽err_add_syms�У�������ȶ����е��ַ�
		sym_t next_sym = use_err_buff_flag ? err_add_syms.front() : s[p];

		next_mov = gen_next_mov(state, next_sym, next);

		switch (next_mov)
		{
		case TableAction_t::ACTION_READ:	// �ƽ�
			if (use_err_buff_flag)
			{
				sym_stack.push(err_add_syms.front());
				err_add_syms.pop();
				attr_stack.push(Attribute());
			}
			else {
				sym_stack.push(s[p]);
				if (s[p] == "num")
				{
					Attribute tmp;
					tmp.value = dynamic_cast<Num*>(tokens[p])->value;
					tmp.line = tokens[p]->line;
					attr_stack.push(tmp);
				}
				else if (s[p] == "id")
				{
					Attribute tmp;
					tmp.lexeme = dynamic_cast<Word*>(tokens[p])->lexeme;
					tmp.line = tokens[p]->line;
					attr_stack.push(tmp);
				}
				else {
					Attribute tmp;
					tmp.line = tokens[p]->line;

					attr_stack.push(tmp);
				}
				++p;
			}
			state_stack.push(next);
			if (next_sym == "{")	// ����飬ѹ��һ���µķ��ű�
			{
				sym_table.push_back(SymTable());
			}
			else if (next_sym == "}")	// �˳��飬����һ�����ű�
			{
				sym_table.pop_back();
			}
			break;
		case TableAction_t::ACTION_REDUCE:	// ��Լ
		{
			Production production = grammer.productions[next];	// ��ԼʱҪʹ�õĲ���ʽ
			int next_state = 0;

			production_action(next, _);	// ��ɲ���ʽ��Ӧ�����嶯��

			// ����ǿչ��򣬾Ͱ� [A->$] ��Ϊ [A->] ����ʽ
			if (production.get_right().size() == 1 && production.get_right()[0] == Production::epsilon)
			{
				production = Production(production.get_left(), vector<sym_t>());
			}

			for (size_t i = 0; i != production.get_right().size(); ++i)	// ����ջ��״̬ջ������Ӧ��Ԫ��
			{
				state_stack.pop();
				sym_stack.pop();
			}
			sym_stack.push(production.get_left());	// ��Լ�õ��ķ��ս�������ջ
			// ��GOTO������һ��״̬
			if (TableAction_t::GOTO == gen_next_mov(state_stack.top(), production.get_left(), next_state))
			{
				state_stack.push(next_state);
			}
			else {	// ERR
				ostringstream err_msg;

				err_msg << "Line" << tokens[p]->line << ": " << endl;
				err_msg << "state " << state_stack.top() << " can't accept " << production.get_left() << " !" << endl;
				err_msg << "??????????? from GOTO" << endl;	// ����Ӧ�ò������Ŷ�
				throw runtime_error(err_msg.str());
			}

			if (verbose)
			{
				os << production << endl << "          ����ջ: ";
				stack<sym_t> s = sym_stack;
				while (!s.empty())
				{
					os << s.top() << " ";
					s.pop();
				}
				cout << endl;
			}

			break;
		}
		case TableAction_t::ACC:	// accept
			if (verbose)
			{
				cout << grammer.productions[0] << endl << "ACC!" << endl;
			}
			production_action(0, _);	// ��ɲ���ʽ��Ӧ�����嶯��
			acc_flag = true;
			break;
		default:		// err
			// �������buffer����֮ǰӦ�ö����������
			err_proc(s, p, tokens, warning_msg);
			break;
		}
	}
	if (warning_num == 0)
	{
		warning_msg << "No Warning!" << endl;
	}
	else {
		warning_msg << endl << warning_num << " Warning(s)!";
	}

	warning_msg << endl << "************************************" << endl;

	os << warning_msg.str() << endl;
	os << "��Ԫʽ: " << endl;
	print_quads(os);
}

// n Ϊ����ʽ���
void LR1Processor::production_action(size_t n, ostream& warning_msg)
{
	Attribute res;	// ��Լ֮�����ʽ�󲿵�����
	vector<Attribute> attr_cache;	// ����һ�µ���������
	Production production = grammer.productions[n];	// ��ԼʱҪʹ�õĲ���ʽ
	Attribute::chain_t Attribute::* pchain;
	Id id;
	SymTable* now_sym_table = nullptr;
	int warning_num = 0;

	// ���û�з��ű��ˣ�˵����Լ������ʼ�ͽ����ĳ���� { decls stmts }���Ժ�Ҳ����Ҫ���ű���
	if (sym_table.size() != 0)
	{
		now_sym_table = &(sym_table.back());	// ��ǰ���Ӧ�ķ��ű�
	}

	// ����ǿչ��򣬾Ͱ� [A->$] ��Ϊ [A->] ����ʽ
	if (production.get_right().size() == 1 && production.get_right()[0] == Production::epsilon)
	{
		production = Production(production.get_left(), vector<sym_t>());
	}

	for (size_t i = 0; i != production.get_right().size(); ++i)	// ����ջ������Ӧ��Ԫ��
	{
		attr_cache.push_back(attr_stack.top());
		attr_stack.pop();
	}

	if (attr_cache.size() != 0)
	{
		res.line = attr_cache[attr_cache.size() - 1].line;
	}

	switch (n)	// n Ϊ����ʽ���
	{
	case 0:
		// program->block
		pchain = &Attribute::chain;	// �������һ����
		attr_cache[0].backpatch(pchain, nxq, quads);
		quads.push_back(Quad("End", "_", "_", "_"));	// �������һ������ʽ
		++nxq;
		break;
	case 1:		// block -> { decls stmts }
		res.chain = attr_cache[1].chain;
		break;
	case 2:		// decls -> decl ; decls
		break;
	case 3:		// decls -> $
		break;
	case 4:		// decl -> decl , id
		res.type = attr_cache[2].type;
		if (!now_sym_table->insert(Id(attr_cache[0].lexeme, res.type)))
		{
			// �ض���
			string err_msg = "Line" + std::to_string(res.line) + ": ";
			throw runtime_error(err_msg + "multiple definition!");
		}
		break;
	case 5:		// decl -> int id
		res.type = Id::Type::INT;
		if (!now_sym_table->insert(Id(attr_cache[0].lexeme, res.type)))
		{
			// �ض���
			string err_msg = "Line" + std::to_string(res.line) + ": ";
			throw runtime_error(err_msg + "multiple definition!");
		}
		break;
	case 6:		// decl -> real id
		res.type = Id::Type::REAL;
		if (!now_sym_table->insert(Id(attr_cache[0].lexeme, res.type)))
		{
			// �ض���
			string err_msg = "Line" + std::to_string(res.line) + ": ";
			throw runtime_error(err_msg + "multiple definition!");
		}
		break;
	case 7:		// stmts -> LS stmt
		res.chain = attr_cache[0].chain;
		break;
	case 8:		// stmts -> $
		break;
	case 9:		// LS -> stmts
		pchain = &Attribute::chain;
		attr_cache[0].backpatch(pchain, nxq, quads);
		break;
	case 10:		// stmt -> C stmt
		pchain = &Attribute::chain;
		res.merge(pchain, attr_cache[0].chain, attr_cache[1].chain);
		break;
	case 11:		// stmt -> TP stmt
		pchain = &Attribute::chain;
		res.merge(pchain, attr_cache[0].chain, attr_cache[1].chain);
		break;
	case 12:		// C -> if bool then
		pchain = &Attribute::TC;
		attr_cache[1].backpatch(pchain, nxq, quads);
		res.chain = attr_cache[1].FC;
		break;
	case 13:		// TP -> C stmt else
	{
		Attribute::chain_t q = { nxq };
		quads.push_back(Quad("j", "_", "_", std::to_string(0)));
		++nxq;
		pchain = &Attribute::chain;
		attr_cache[2].backpatch(pchain, nxq, quads);
		pchain = &Attribute::chain;
		res.merge(pchain, attr_cache[1].chain, q);
		break;
	}
	case 14:		// stmt -> Wd stmt
		pchain = &Attribute::chain;
		attr_cache[0].backpatch(pchain, attr_cache[1].quad, quads);
		quads.push_back(Quad("j", "_", "_", std::to_string(attr_cache[1].quad)));
		++nxq;
		res.chain = attr_cache[1].chain;
		break;
	case 15:		// stmt -> Ww bool ;
		pchain = &Attribute::TC;
		attr_cache[1].backpatch(pchain, attr_cache[2].quad, quads);
		res.chain = attr_cache[1].FC;
		break;
	case 16:		// Wd -> W bool D
		pchain = &Attribute::TC;
		attr_cache[1].backpatch(pchain, nxq, quads);
		res.chain = attr_cache[1].FC;
		res.quad = attr_cache[2].quad;
		break;
	case 17:		// Ww -> D stmt W
		pchain = &Attribute::chain;
		attr_cache[1].backpatch(pchain, nxq, quads);
		res.quad = attr_cache[2].quad;
		break;
	case 18:		// W -> while
		res.quad = nxq;
		break;
	case 19:		// D -> do
		res.quad = nxq;
		break;
	case 20:		// stmt -> id = expr ;
		res.chain = Attribute::chain_t();

		if (get_info(attr_cache[3].lexeme, id))
		{
			attr_cache[3].type = id.type;
			if (attr_cache[1].type != attr_cache[3].type)
			{
				string new_place = "t" + std::to_string(tmp_idx++);
				if (attr_cache[3].type == Id::Type::INT)
				{
					// �Զ�����ת�������ܻ���ʧ����
					warning_msg << endl << "Line" << std::to_string(res.line) << ": " << endl
						<< "Warning " << warning_num++ << ": Real to int!" << endl;

					// real to int
					quads.push_back(Quad("rtoi", attr_cache[1].place, "_", new_place));
				}
				else {
					// int to real
					quads.push_back(Quad("itor", attr_cache[1].place, "_", new_place));
				}
				++nxq;
				attr_cache[1].place = new_place;
			}
			quads.push_back(Quad("=", attr_cache[1].place, "_", "ENTRY(" + attr_cache[3].lexeme + ")"));
			++nxq;
			set_val(attr_cache[3].lexeme, attr_cache[1].value);
		}
		else {
			string err_msg = "Line" + std::to_string(res.line) + ": "
				+ "Undefined variant " + attr_cache[3].lexeme + " !";
			throw runtime_error(err_msg);
		}

		break;
	case 21:		// stmt -> block
		res.chain = attr_cache[0].chain;
		break;
	case 22:		// expr -> expr + term
		if (attr_cache[0].type != attr_cache[2].type)
		{
			string new_place = "t" + std::to_string(tmp_idx++);
			if (attr_cache[2].type == Id::Type::INT)
			{
				// int to real
				quads.push_back(Quad("itor", attr_cache[2].place, "_", new_place));
				attr_cache[2].place = new_place;
			}
			else {
				// int to real
				quads.push_back(Quad("itor", attr_cache[0].place, "_", new_place));
				attr_cache[0].place = new_place;
			}
			res.type = Id::Type::REAL;
			++nxq;
		}
		else {
			res.type = attr_cache[0].type;
		}

		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("+", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = attr_cache[2].value + attr_cache[0].value;
		break;
	case 23:		// expr -> expr - term
		if (attr_cache[0].type != attr_cache[2].type)
		{
			string new_place = "t" + std::to_string(tmp_idx++);
			if (attr_cache[2].type == Id::Type::INT)
			{
				// int to real
				quads.push_back(Quad("itor", attr_cache[2].place, "_", new_place));
				attr_cache[2].place = new_place;
			}
			else {
				// int to real
				quads.push_back(Quad("itor", attr_cache[0].place, "_", new_place));
				attr_cache[0].place = new_place;
			}
			res.type = Id::Type::REAL;
			++nxq;
		}
		else {
			res.type = attr_cache[0].type;
		}

		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("-", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = attr_cache[2].value - attr_cache[0].value;
		break;
	case 24:		// expr -> term
		res.type = attr_cache[0].type;
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 25:		// term -> term * unary
		if (attr_cache[0].type != attr_cache[2].type)
		{
			string new_place = "t" + std::to_string(tmp_idx++);
			if (attr_cache[2].type == Id::Type::INT)
			{
				// int to real
				quads.push_back(Quad("itor", attr_cache[2].place, "_", new_place));
				attr_cache[2].place = new_place;
			}
			else {
				// int to real
				quads.push_back(Quad("itor", attr_cache[0].place, "_", new_place));
				attr_cache[0].place = new_place;
			}
			res.type = Id::Type::REAL;
			++nxq;
		}
		else {
			res.type = attr_cache[0].type;
		}

		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("*", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = attr_cache[2].value * attr_cache[0].value;
		break;
	case 26:		// term -> term / unary
		if (attr_cache[0].type != attr_cache[2].type)
		{
			string new_place = "t" + std::to_string(tmp_idx++);
			if (attr_cache[2].type == Id::Type::INT)
			{
				// int to real
				quads.push_back(Quad("itor", attr_cache[2].place, "_", new_place));
				attr_cache[2].place = new_place;
			}
			else {
				// int to real
				quads.push_back(Quad("itor", attr_cache[0].place, "_", new_place));
				attr_cache[0].place = new_place;
			}
			res.type = Id::Type::REAL;
			++nxq;
		}
		else {
			res.type = attr_cache[0].type;
		}

		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("/", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		if (attr_cache[0].value == 0)
		{
			string err_msg = "Line" + std::to_string(res.line) + ": "
				+ "Err: Divided by zero!";
			throw runtime_error(err_msg);
		}
		else {
			res.value = attr_cache[2].value / attr_cache[0].value;
		}
		break;
	case 27:		// term -> unary
		res.type = attr_cache[0].type;
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 28:		// unary -> - unary
		res.type = attr_cache[0].type;
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("@", attr_cache[0].place, "_", res.place));
		++nxq;
		res.value = -attr_cache[0].value;
		break;
	case 29:		// unary -> factor
		res.type = attr_cache[0].type;
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 30:		// factor -> ( expr )
		res.type = attr_cache[1].type;
		res.place = attr_cache[1].place;
		res.value = attr_cache[1].value;
		break;
	case 31:		// factor -> id
		if (!get_info(attr_cache[0].lexeme, id))
		{
			string err_msg = "Line" + std::to_string(res.line) + ": "
				+ "Undefined variant " + attr_cache[0].lexeme + " !";
			throw runtime_error(err_msg);
		}
		else {
			res.place = "ENTRY(" + attr_cache[0].lexeme + ")";
			res.type = id.type;
			res.value = id.value;
		}
		break;
	case 32:		// factor -> num
		res.type = Id::Type::REAL;
		res.place = std::to_string(attr_cache[0].value);
		res.value = attr_cache[0].value;
		break;
	case 33:		// bool -> boolor join
		res.place = attr_cache[1].place + " || " + attr_cache[0].place;

		pchain = &Attribute::TC;
		res.merge(pchain, attr_cache[1].TC, attr_cache[0].TC);
		res.FC = attr_cache[0].FC;
		break;
	case 34:		// bool -> join
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		res.TC = attr_cache[0].TC;
		res.FC = attr_cache[0].FC;
		break;
	case 35:		// join -> joinand boolterm 
		res.place = attr_cache[1].place + " && " + attr_cache[0].place;

		pchain = &Attribute::FC;
		res.merge(pchain, attr_cache[1].FC, attr_cache[0].FC);
		res.TC = attr_cache[0].TC;
		break;
	case 36:		// join -> boolterm
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		res.TC = attr_cache[0].TC;
		res.FC = attr_cache[0].FC;
		break;
	case 37:		// boolor -> bool or
		res.place = attr_cache[1].place;
		res.value = attr_cache[1].value;
		res.TC = attr_cache[1].TC;
		pchain = &Attribute::FC;
		attr_cache[1].backpatch(pchain, nxq, quads);
		break;
	case 38:		// joinand -> join and
		res.place = attr_cache[1].place;
		res.value = attr_cache[1].value;
		res.FC = attr_cache[1].FC;
		pchain = &Attribute::TC;
		attr_cache[1].backpatch(pchain, nxq, quads);
		break;
	case 39:		// boolterm -> equality
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		res.TC = { nxq };
		res.FC = { nxq + 1 };
		quads.push_back(Quad("jtrue", attr_cache[0].place, "_", std::to_string(0)));
		++nxq;
		quads.push_back(Quad("j", "_", "_", std::to_string(0)));
		++nxq;
		break;
	case 40:		// equality -> equality == rel
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("==", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = (attr_cache[2].value == attr_cache[0].value);
		break;
	case 41:		// equality -> equality != rel
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("!=", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = (attr_cache[2].value != attr_cache[0].value);
		break;
	case 42:		// equality -> rel
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 43:		// equality -> true
		res.place = "true";
		res.value = 1;
		break;
	case 44:		// equality -> false
		res.place = "false";
		res.value = 0;
		break;
	case 45:		// rel -> rel < relexpr
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("<", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = (attr_cache[2].value < attr_cache[0].value);
		break;
	case 46:		// rel -> rel <= relexpr
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("<=", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = (attr_cache[2].value <= attr_cache[0].value);
		break;
	case 47:		// rel -> rel >= relexpr
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad(">=", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = (attr_cache[2].value >= attr_cache[0].value);
		break;
	case 48:		// rel -> rel > relexpr
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad(">", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = (attr_cache[2].value > attr_cache[0].value);
		break;
	case 49:		// rel -> relexpr
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 50:		// relexpr -> relexpr + relterm 
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("+", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = attr_cache[2].value + attr_cache[0].value;
		break;
	case 51:		// relexpr -> relexpr - relterm
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("-", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = attr_cache[2].value - attr_cache[0].value;
		break;
	case 52:		// relexpr -> relterm
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 53:		// relterm -> relterm * relunary
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("*", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		res.value = attr_cache[2].value * attr_cache[0].value;
		break;
	case 54:		// relterm -> relterm / relunary
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("/", attr_cache[2].place, attr_cache[0].place, res.place));
		++nxq;
		if (attr_cache[0].value == 0)
		{
			string err_msg = "Line" + std::to_string(res.line) + ": "
				+ "Err: Divided by zero!";
			throw runtime_error(err_msg);
		}
		else {
			res.value = attr_cache[2].value / attr_cache[0].value;
		}
		break;
	case 55:		// relterm -> relunary
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 56:		// relunary -> ! relunary
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("!", attr_cache[0].place, "_", res.place));
		++nxq;
		res.value = (attr_cache[0].value == 0) ? 1 : 0;
		break;
	case 57:		// relunary -> - relunary
		res.place = "t" + std::to_string(tmp_idx++);
		quads.push_back(Quad("@", attr_cache[0].place, "_", res.place));
		++nxq;
		res.value = -attr_cache[0].value;
		break;
	case 58:		// relunary -> relfactor
		res.place = attr_cache[0].place;
		res.value = attr_cache[0].value;
		break;
	case 59:		// relfactor -> ( bool )
		pchain = &Attribute::TC;
		attr_cache[1].backpatch(pchain, nxq, quads);
		pchain = &Attribute::FC;
		attr_cache[1].backpatch(pchain, nxq, quads);
		res.place = attr_cache[1].place;
		res.value = attr_cache[1].value;
		break;
	case 60:		// relfactor -> id
		if (!get_info(attr_cache[0].lexeme, id))
		{
			string err_msg = "Line" + std::to_string(res.line) + ": "
				+ "Undefined variant " + attr_cache[0].lexeme + " !";
			throw runtime_error(err_msg);
		}
		else {
			res.place = "ENTRY(" + attr_cache[0].lexeme + ")";
			res.value = id.value;
		}
		break;
	case 61:		// relfactor -> num
		res.place = std::to_string(attr_cache[0].value);
		res.value = attr_cache[0].value;
		break;
	default:
		cout << "Err!" << endl;
		break;
	}
	attr_stack.push(res);
}

bool LR1Processor::get_info(const std::string& name, Id& id) const
{
	// �ӵ�ǰ�鿪ʼ�����������Ѱ�ұ���
	for (auto it = sym_table.rbegin(); it != sym_table.rend(); ++it)
	{
		if (it->get_info(name, id))
		{
			return true;
		}
	}
	return false;
}

bool LR1Processor::set_val(const std::string& name, double val)
{
	Id id;
	// �ӵ�ǰ�鿪ʼ�����������Ѱ�ұ���
	for (auto it = sym_table.rbegin(); it != sym_table.rend(); ++it)
	{
		if (it->table.find(name) != it->table.end())
		{
			(it->table)[name].value = val;
			return true;
		}
	}
	return false;
}

std::ostream& LR1Processor::print_quads(std::ostream& os) const
{
	for (size_t i = 1; i != quads.size(); ++i)
	{
		os << i << ": " << quads[i] << endl;
	}
	return os;
}



