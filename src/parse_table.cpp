#include "parse_table.h"

#include <iostream>


namespace parse_table {


using std::cout;
using std::dynamic_pointer_cast;
using std::endl;
using std::make_shared;
using std::to_string;



ParsingTableGenerator::ParsingTableGenerator () {
}

void ParsingTableGenerator::addSymbol(const shared_ptr<Nonterminal> &non, bool is_start_symbol/*=false*/) {
	symbols_.push_back(non);

	if (is_start_symbol == true)
		start_symbol_ = non;
}

Configuration::Configuration (
		const shared_ptr<Nonterminal> &left_side,
		const vector<RuleElem> &rule,
		int idx_after_cursor,
		const set<shared_ptr<Terminal>> &lookahead)
:left_side_(left_side),
 rule_(rule),
 idx_after_cursor_(idx_after_cursor),
 lookahead_(lookahead) {

}

void Configuration::print () {
	cout << left_side_->name << " -> ";
	for (int ri = 0; ri < rule_.size(); ++ri) {
		auto &re = rule_[ri];
		if (ri == idx_after_cursor_)
			cout << ".";
		cout << re.termnon->name;
		if (ri < rule_.size() - 1)
			cout << " ";
	}

	if (idx_after_cursor_ >= rule_.size())
		cout << ".";

	cout << "  , ";
	int li = 0;
	for (auto &t : lookahead_) {
		if (li != 0)
			cout << " / ";
		if (t == null)
			cout << "$";
		else
			cout << t->name;
		++li;
	}

	cout << endl;
}

State::State (
		const vector<shared_ptr<Configuration>> &transited_configs,
		const vector<shared_ptr<Configuration>> &closures)
:transited_configs_(transited_configs),
 closures_(closures) {

}

static void _findFirst (
		const shared_ptr<TerminalBase> &termnon,
		set<shared_ptr<Terminal>> &result,
		set<shared_ptr<TerminalBase>> &history) {
	
	if (termnon->isTerminal()) {
		auto term = dynamic_pointer_cast<Terminal>(termnon);
		result.insert(term);
		return;
	}


	// nonterminal part
	auto non = dynamic_pointer_cast<Nonterminal>(termnon);

	// prevent infinity loop
	if (history.find(non) != history.end())
		return;
	history.insert(termnon);

	// find recursively
	for (auto rule : non->rules) {
		auto &re = rule[0];
		_findFirst(re.termnon, result, history);
	}
}

static void _findFirst(
		const shared_ptr<TerminalBase> &termnon,
		set<shared_ptr<Terminal>> &result) {

	set<shared_ptr<TerminalBase>> history;
	return _findFirst(termnon, result, history);
}

static void _addClosures (
		const FirstMap &first_map,
		const shared_ptr<Configuration> &c,
		set<shared_ptr<Nonterminal>> &non_set,
		vector<shared_ptr<Configuration>> &closures) {
	auto &rule = c->rule();
	auto idx_after_cursor = c->idx_after_cursor();
	if (idx_after_cursor >= rule.size())
		return;

	// find non after cursor
	auto &re = rule[idx_after_cursor];
	if (re.termnon->isTerminal() == true)
		return;
	auto non = dynamic_pointer_cast<Nonterminal>(re.termnon);
	

	// make lookahead
	set<shared_ptr<Terminal>> lookahead;
	if (idx_after_cursor + 1 >= rule.size()) {
		// if next isn't exist, use lookahead
		lookahead = c->lookahead();
	}else {
		// use first(next)
		auto &next = rule[idx_after_cursor + 1].termnon;
		if (next->isTerminal() == true)
			lookahead.insert(dynamic_pointer_cast<Terminal>(next));
		else
			lookahead = first_map.at(dynamic_pointer_cast<Nonterminal>(next));
	}


	// add closures
	if (non_set.find(non) == non_set.end()) {
		// add configs to closures
		non_set.insert(non);
		for (auto &rule : non->rules) {
			closures.push_back(make_shared<Configuration>(non, rule, 0, lookahead));
		}
	}else {
		// merge lookahead
		for (auto &c : closures) {
			if (c->left_side() != non)
				continue;

			c->lookahead().insert(lookahead.begin(), lookahead.end());
		}
	}
}

shared_ptr<State> State::makeState(
		const vector<shared_ptr<Nonterminal>> &all_symbols,
		const FirstMap &first_map,
		const vector<shared_ptr<Configuration>> &transited_configs) {

	// make nonterminal set to prevent Configuration duplicated
	set<shared_ptr<Nonterminal>> non_set;
	for (auto &c : transited_configs)
		non_set.insert(c->left_side());

	// make closures
	vector<shared_ptr<Configuration>> closures;
	for (auto &c : transited_configs)
		_addClosures(first_map, c, non_set, closures);

	// add closures from closures
	for (int ci = 0; ci < closures.size(); ++ci) {
		auto &c = closures[ci];

		_addClosures(first_map, c, non_set, closures);
	}

	return make_shared<State>(transited_configs, closures);
}

void State::print () {
	cout << "## state " << name_ << endl;
	cout << "transited:" << endl;
	for (auto &c : transited_configs_)
		c->print();

	cout << "closures:" << endl;
	for (auto &c : closures_)
		c->print();

	cout << "transition:" << endl;
	for (auto &itr : transition_map_) {
		cout << itr.first->name << " ==> " << itr.second->name() << endl;
	}
	unordered_map<shared_ptr<TerminalBase>, shared_ptr<State>> transition_map_;
}

static void _makeNextTransitionInfoMap (
		const vector<shared_ptr<Configuration>> &configs,
		unordered_map<shared_ptr<TerminalBase>, vector<shared_ptr<Configuration>>> &result) {

	for (auto &c : configs) {
		if (c->idx_after_cursor() >= c->rule().size())
			continue;

		auto &re = c->rule()[c->idx_after_cursor()];

		result[re.termnon].push_back(
				make_shared<Configuration>(
					c->left_side(),
					c->rule(),
					c->idx_after_cursor() + 1,
					c->lookahead()));
	}
}

static void _addStates (
		const vector<shared_ptr<Nonterminal>> &symbols,
		const FirstMap &first_map,
		const shared_ptr<State> &base,
		vector<shared_ptr<State>> &all_states,
		set<shared_ptr<State>> &history) {

	if (history.find(base) != history.end())
		return;
	history.insert(base);

	// make next_map
	unordered_map<shared_ptr<TerminalBase>, vector<shared_ptr<Configuration>>> next_map;
	_makeNextTransitionInfoMap(base->transited_configs(), next_map);
	_makeNextTransitionInfoMap(base->closures()         , next_map);

	// make new state
	for (auto &itr : next_map) {
		auto new_state = State::makeState(symbols, first_map, itr.second);
		base->transition_map()[itr.first] = new_state;
		all_states.push_back(new_state);
	}
}

static bool _areStatesEqual (
		const shared_ptr<State> &state,
		const shared_ptr<State> &other) {

	if (state->transited_configs().size() != other->transited_configs().size())
		return false;

	if (state->closures().size() != other->closures().size())
		return false;

	for (int ci = 0; ci < state->transited_configs().size(); ++ci) {
		auto &state_c = state->transited_configs()[ci];
		auto &other_c = other->transited_configs()[ci];
		if (state_c->left_side()        != other_c->left_side()        ||
			state_c->idx_after_cursor() != other_c->idx_after_cursor() ||
			state_c->rule().size()      != other_c->rule().size())
			return false;

		auto &state_r = state_c->rule();
		auto &other_r = other_c->rule();
		for (int ri = 0; ri < state_r.size(); ++ri) {
			if (state_r[ri].termnon != other_r[ri].termnon)
				return false;
		}
	}

	return true;
}

static void _mergeStates_configs(
		const vector<shared_ptr<Configuration>> &configs,
		const vector<shared_ptr<Configuration>> &other) {
	for (int ci = 0; ci < configs.size(); ++ci) {
		auto &c       = configs[ci];
		auto &other_c = other  [ci];
		c->lookahead().insert(other_c->lookahead().begin(), other_c->lookahead().end());
	}
}

static void _mergeStates (
		vector<shared_ptr<State>> &states,
		vector<shared_ptr<State>> &result) {

	vector<shared_ptr<State>> new_states;
	// merge states
	for (int si = 0; si < states.size(); ++si) {
		auto &s = states[si];
		if (s == null)
			continue;

		new_states.push_back(s);

		for (int other_i = si + 1; other_i < states.size(); ++other_i) {
			auto &other_s = states[other_i];
			if (other_s == null)
				continue;

			if (_areStatesEqual(s, other_s) == false)
				continue;


			// merge
			_mergeStates_configs(s->transited_configs(), other_s->transited_configs());
			_mergeStates_configs(s->closures()         , other_s->closures()         );

			for (auto &sss : states) {
				if (sss == null)
					continue;

				auto &transition_map = sss->transition_map();
				for (auto &itr : transition_map) {
					if (itr.second == other_s)
						transition_map[itr.first] = s;
				}
			}

			states[other_i] = null;
		}
	}

	result = new_states;
}

shared_ptr<ParseTable> ParsingTableGenerator::generateTable () {

	if (start_symbol_ == null) {
		//TODO err: you have to set start_symbol
		return null;
	}

	// make first map
	FirstMap first_map;
	for (auto &non : symbols_) {
		auto &first = first_map[non];
		_findFirst(non, first);
	}

	auto result = make_shared<ParseTable>();

	// make s_prime
	auto s_prime = make_shared<Nonterminal>("S\'");
	s_prime->rules.push_back({ RuleElem(start_symbol_, RuleElem::ANY) });

	// make start state
	set<shared_ptr<Terminal>> start_lookahead{ null };
	vector<shared_ptr<Configuration>> start_configs {
		make_shared<Configuration>(s_prime, s_prime->rules[0], 0, start_lookahead)
	};

	auto start_state = State::makeState(symbols_, first_map, start_configs);
	states_.push_back(start_state);

	// add states
	set<shared_ptr<State>> history;
	for (int si = 0; si < states_.size(); ++si) {
		auto &s = states_[si];
		s->name("State " + to_string(si));
		_addStates(symbols_, first_map, start_state, states_, history);
	}

	// merge states
	_mergeStates(states_, states_);

	// print states
	for (auto &s : states_) {
		cout << "################################" << endl;
		s->print();
	}

	// TODO

	return result;
}

}

