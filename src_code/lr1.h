#pragma once

#include "token.h"
#include "symbols.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <stack>
#include <queue>

// ��Ԫʽ
struct Quad {
	friend std::ostream& operator<< (std::ostream& os, const Quad& q);

	Quad() = default;
	Quad(std::string o, std::string l, std::string r, std::string d)
		: opt(o), lhs(l), rhs(r), dest(d) {}

	std::string opt;
	std::string lhs;
	std::string rhs;
	std::string dest;
};

// ����һЩ�﷨��Ԫ������
// ��Ϊ�����Դ���Ժܼ򵥣����Ժ��٣�Ϊ��ʡ���Ҿ�ֱ�Ӱ��������Զ����嵽һ��������
struct Attribute {
	using chain_t = std::unordered_set<size_t>;	// ���ڻ������Ԫʽ��ַ��

	Attribute() = default;
	~Attribute() = default;

	// ����Ϊ��������
	chain_t chain;	// �����������йص�����
	chain_t TC;
	chain_t FC;
	size_t quad;

	Id::Type type;		// ��������
	double value;			// ��������ʽ��ֵ
	std::string place;		// ��ʱ������

	std::string lexeme;	// ����

	size_t line;	// �﷨��Ԫ������

	// ����
	void backpatch(chain_t Attribute::* pchain, size_t n, std::vector<Quad> & quads)
	{
		for (auto& i : this->*pchain)
		{
			quads[i].dest = std::to_string(n);
		}
	}
	void merge(chain_t Attribute::*pchain, const chain_t& chain1, const chain_t& chain2)
	{
		(this->*pchain).insert(chain1.begin(), chain1.end());
		(this->*pchain).insert(chain2.begin(), chain2.end());
	}
};


// ����ʽ
class Production {
	friend std::ostream& operator<<(std::ostream& out, Production& p);
public:
	using sym_t = std::string;

	Production() = default;
	Production(const sym_t&l, const std::vector<sym_t> &r) : left(l), right(r) {}
	virtual ~Production() {};
	static const sym_t epsilon;	// ����ʽ�еĿչ���

	size_t len() const { return right.size(); }	// ���ز���ʽ�Ҳ��ĳ���

	std::vector<sym_t> get_right() const{ return right; }	// ���ز���ʽ�Ҳ�
	sym_t get_left() const { return left; }	// ���ز���ʽ��
	bool input_production(const std::string& p);
private:
	sym_t left = sym_t();	// ����ʽ��
	std::vector<sym_t> right;	// ����ʽ�Ҳ�
};

// �ķ�
class Grammer {
	friend class LR1Processor;
	friend class LR1DFA;
	friend std::ostream& operator<<(std::ostream& out, Grammer& g);
	friend std::istream& operator>>(std::istream& in, Grammer& g);
public:
	using sym_t = Production::sym_t;
	static const sym_t end_of_sentence;		// ���ӽ�β

	Grammer() : T{ Grammer::end_of_sentence, Production::epsilon } {}	// '#' ��ʾ���ӽ�β��'$' ��ʾ�չ���
	~Grammer() = default;

	std::ostream& output_first_set(std::ostream& os) const;	// �����õ� FIRST ��
	std::unordered_set<sym_t> first_set_string(const std::vector<sym_t>& s) const;	// ��һ�����ŵ� FIRST ��
	
private:
	std::vector<Production> productions;	// ����ʽ
	std::unordered_set<sym_t> T;	// �ս��
	std::unordered_set<sym_t> N;	// ���ս��
	std::vector<sym_t> TN;	// ���з��� (�ս��+���ս��)
	std::unordered_map<sym_t, std::unordered_set<sym_t>> first;	// �ս���ͷ��ս���� FIRST ��

	// �ж�ĳ������ʽ��һ�������Ƿ�Ϊ�ս��
	bool first_is_T(const Production&p) const {
		return !(T.find(p.get_right()[0]) == T.end());
	}	
	bool first_is_T(const sym_t& c) const {
		return !(T.find(c) == T.end());
	}
	void first_set();	// ���ķ��е������ս���ͷ��ս���� FIRST ��
};

// LR1 ��Ŀ
// hashable
class LR1Item : public Production {
	friend class LR1DFA;
	friend class LR1Processor;
	friend bool operator==(const LR1Item &item1, const LR1Item &item2);
	friend struct std::hash<LR1Item>;
public:
	LR1Item() = default;
	LR1Item(const Production& p, const sym_t& n, const size_t& o) : Production(p), next(n), loc(o) {}
	LR1Item(const sym_t& l, const std::vector<sym_t>& r, const sym_t& n, const size_t &o) : Production(l, r), next(n), loc(o){}
private:
	sym_t next;	// ��ǰ������
	size_t loc;	// ��Ŀ�е��λ��
};

namespace std {
	template<>
	struct hash<LR1Item>	// ������ LR1 ��Ŀ�� hash ��
	{
		using result_type = size_t;
		using argument_type = LR1Item;
		size_t operator()(const LR1Item& item) const
		{
			auto right = item.get_right();
			size_t hash_res = hash<LR1Item::sym_t>()(item.next) ^
								hash<size_t>()(item.loc) ^
								hash< LR1Item::sym_t>()(item.get_left());

			for (const auto& sym : right)
			{
				hash_res ^= hash<LR1Item::sym_t>()(sym);
			}

			return  hash_res;
		}
	};
}

class LR1DFA {
	friend class LR1Processor;
public:
	using sym_t = Production::sym_t;
	using LR1_items_t = std::unordered_set<LR1Item>;		// LR1 ��Ŀ�� hashable
	
	LR1DFA(const Grammer& g = Grammer()) : grammer(g){}
	std::ostream& print_lr1_items(const LR1_items_t& items, std::ostream& os) const;
private:
	LR1_items_t closure(const LR1_items_t& items);	// �� LR1 �����հ�
	LR1_items_t go_to(const LR1_items_t& items, sym_t c);	// GOTO �������� items ����� c ֮�󵽴���һ�����������������������У������һ��
	Grammer grammer;
};

namespace std {
	template<>
	struct hash<LR1DFA::LR1_items_t>	// ������ LR1 ��Ŀ���� hash ��
	{
		using result_type = size_t;
		using argument_type = LR1DFA::LR1_items_t;
		size_t operator()(const LR1DFA::LR1_items_t& items) const
		{
			size_t hash_res = 0;

			for (const auto& item : items)
			{
				hash_res ^= hash<LR1Item>()(item);
			}

			return  hash_res;
		}
	};
}

/*
* �����������ķ����� LR1 ������������ '#' ��β�ľ���
* 
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
class LR1Processor {
	friend std::ostream& operator<<(std::ostream& out, LR1Processor& g);
	friend struct Attribute;
public:
	using sym_t = Production::sym_t;
	using ambiguity_term_t = std::pair<int, sym_t>;	// ��¼LR1���������ж����Ե�λ��

	LR1Processor(std::string file_name = "grammer.txt");
	std::vector<std::unordered_map<sym_t, std::string>> action_table;	// ACTION��
	std::vector<std::unordered_map<sym_t, std::string>> goto_table;	// GOTO��
	std::vector<LR1DFA::LR1_items_t> lr1_sets;	// LR1 �淶���
	std::list<ambiguity_term_t> ambiguity_terms;	// ��¼LR1���������ж����Ե�λ��

	void solve_ambiguity_if();	// ��� if �Ķ���������
	void solve_ambiguity();		// ��������ķ��Ķ���������
	std::ostream& print_lr1_table(std::ostream& os) const;	// ��ӡ LR1 ������
	std::ostream& print_lr1_sets(std::ostream& os) const;	// ��ӡ LR1 �淶���
	std::ostream& print_quads(std::ostream& os) const;	// ��ӡ��Ԫʽ
	
	void driver(std::vector<sym_t> &s, std::vector<Token*> tokens, bool verbose = true, std::ostream& os = std::cout);		// LR1 ������������ / �ܿس���
	bool get_info(const std::string& name, Id& id) const;
	bool set_val(const std::string& name, double val);
private:
	std::ifstream in;	// �����ķ����ļ�
	Grammer grammer;	// �ķ�
	LR1DFA dfa;
	enum TableAction_t : unsigned {	// Action / Goto ���еĶ���
		ACTION_READ = 0,	// �ƽ�
		ACTION_REDUCE,	// ��Լ
		GOTO,			// GOTO��
		ACC,			// accept
		ERR,			// ����
	};

	std::queue<sym_t> err_add_syms;	// �ɴ���ָ��������ķ���
	void construct_table();	// ���� LR1 ������
	TableAction_t gen_next_mov(const int& state, sym_t next_sym, int &next) const;
	std::ostream& err_proc(std::vector<sym_t>& s, size_t &p, const std::vector<Token*> &tokens, std::ostream& os = std::cerr);		// ������
	void production_action(size_t n, std::ostream& warning_msg);
	void init_state();	// ���»س�ʼ״̬

	std::stack<int> state_stack;		// ״̬ջ
	std::stack<sym_t> sym_stack;		// ����ջ
	std::stack<Attribute> attr_stack;	// ����ջ
	std::vector<SymTable> sym_table;			// ���ű�
	size_t nxq = 1;	// ��һ����Ԫʽ��ţ���һ����Ԫʽ��ַΪ 1
	std::vector<Quad> quads = { Quad() };	// ��Ԫʽ
	size_t tmp_idx = 0;	// �������ʱ���������
	size_t warning_num = 0;	// ������ Warning �ĸ���
};

std::ostream& operator<<(std::ostream& out, Production& p);
std::ostream& operator<<(std::ostream& out, Grammer& g);
std::ostream& operator<<(std::ostream& out, LR1Processor& l);
std::istream& operator>>(std::istream& in, Grammer& g);
bool operator==(const LR1Item &item1, const LR1Item &item2);
std::ostream& operator<< (std::ostream& os, const Quad& q);
