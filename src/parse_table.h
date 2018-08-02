#ifndef PAW_PRINT_PARSE_TABLE
#define PAW_PRINT_PARSE_TABLE

#include <memory>
#include <set>
#include <vector>

#include "token.h"
#include "node.h"

#include "defines.h"

namespace parse_table {

using std::set;
using std::shared_ptr;
using std::vector;

using FirstMap = unordered_map<shared_ptr<Nonterminal>, set<shared_ptr<Terminal>>>;


class Configuration {
public:
	PAW_GETTER(const shared_ptr<Nonterminal>&, left_side)
	PAW_GETTER(const Rule&, rule)
	PAW_GETTER(int, idx_after_cursor)

	inline set<shared_ptr<Terminal>>& lookahead () { return lookahead_; }


	Configuration (
			const shared_ptr<Nonterminal> &left_side,
			const Rule &rule,
			int idx_after_cursor,
			const set<shared_ptr<Terminal>> &lookahead);

	void print ();


private:
	shared_ptr<Nonterminal> left_side_;
	const Rule &rule_;
	int idx_after_cursor_;
	set<shared_ptr<Terminal>> lookahead_; // null means end($)
};

class State {
public:
	static shared_ptr<State> makeState(
			const vector<shared_ptr<Nonterminal>> &all_terminals,
			const FirstMap &first_map,
			const vector<shared_ptr<Configuration>> &transited_configs);

	PAW_GETTER_SETTER(const string &, name)
	PAW_GETTER(const vector<shared_ptr<Configuration>>&, transited_configs)
	PAW_GETTER(const vector<shared_ptr<Configuration>>&, closures)
	inline unordered_map<shared_ptr<TerminalBase>, shared_ptr<State>>& transition_map () { return transition_map_; }

	State (
			const vector<shared_ptr<Configuration>> &transited_configs,
			const vector<shared_ptr<Configuration>> &closures);

	void print ();


private:
	string name_;
	vector<shared_ptr<Configuration>> transited_configs_;
	vector<shared_ptr<Configuration>> closures_;
	unordered_map<shared_ptr<TerminalBase>, shared_ptr<State>> transition_map_;
};

class ParsingTable {
public:
	class ActionInfo {
	public:
		enum Action {
			NONE,
			SHIFT,
			REDUCE,
			GOTO,
			ACCEPT,
		};

		Action action;
		int idx;

		ActionInfo ();
		ActionInfo (Action action, int idx);
	};

	vector<const Rule*> rules;
	vector<unordered_map<shared_ptr<TerminalBase>, ActionInfo>> action_info_map_list;

	ParsingTable (
			const vector<shared_ptr<Nonterminal>> &symbols,
			const shared_ptr<Nonterminal> &start_symbol,
			const vector<shared_ptr<State>> &states);
};

class ParsingTableGenerator {
public:
	PAW_GETTER(const shared_ptr<Nonterminal>&, start_symbol)

	ParsingTableGenerator ();

	void addSymbol (const shared_ptr<Nonterminal> &non, bool is_start_symbol = false);

	shared_ptr<ParsingTable> generateTable ();

private:
	vector<shared_ptr<Nonterminal>> symbols_;
	shared_ptr<Nonterminal> start_symbol_;
	vector<shared_ptr<State>> states_;
};

#include "undefines.h"
}

#endif