#include "parse_table.h"

#include <iomanip>
#include <iostream>


namespace parse_table {


using std::cout;
using std::dynamic_pointer_cast;
using std::endl;
using std::make_shared;
using std::setfill;
using std::setw;
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
		const Rule &rule,
		int idx_after_cursor,
		const set<RuleElem> &lookahead)
:left_side_(left_side),
 rule_(rule),
 idx_after_cursor_(idx_after_cursor),
 lookahead_(lookahead) {

}

void Configuration::print () {
	cout << left_side_->name << " -> ";
	for (int ri = 0; ri < rule_.right_side.size(); ++ri) {
		auto &re = rule_.right_side[ri];
		if (ri == idx_after_cursor_)
			cout << ".";
		cout << re.termnon->name;
		if (ri < rule_.right_side.size() - 1)
			cout << " ";
	}

	if (idx_after_cursor_ >= rule_.right_side.size())
		cout << ".";

	cout << "  , ";
	int li = 0;
	for (auto &re : lookahead_) {
		if (li != 0)
			cout << " / ";
		if (re.termnon == null)
			cout << "$";
		else
			cout << re.termnon->name;
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
		const RuleElem &re,
		set<RuleElem> &result,
		set<RuleElem> &history) {
	
	if (re.termnon->isTerminal()) {
		result.insert(re);
		return;
	}


	// nonterminal part
	auto non = dynamic_pointer_cast<Nonterminal>(re.termnon);

	// prevent infinity loop
	if (history.find(re) != history.end())
		return;
	history.insert(re);

	// find recursively
	for (auto rule : non->rules) {
		auto &first_re = rule.right_side[0];
		_findFirst(first_re, result, history);
	}
}

static void _findFirst(
		const RuleElem &re,
		set<RuleElem> &result) {

	set<RuleElem> history;
	return _findFirst(re, result, history);
}

static void _addClosures (
		const FirstMap &first_map,
		const shared_ptr<Configuration> &c,
		set<shared_ptr<Nonterminal>> &non_set,
		vector<shared_ptr<Configuration>> &closures) {
	auto &rule = c->rule();
	auto idx_after_cursor = c->idx_after_cursor();
	if (idx_after_cursor >= rule.right_side.size())
		return;

	// find non after cursor
	auto &re = rule.right_side[idx_after_cursor];
	if (re.termnon->isTerminal() == true)
		return;
	auto non = dynamic_pointer_cast<Nonterminal>(re.termnon);
	

	// make lookahead
	set<RuleElem> lookahead;
	if (idx_after_cursor + 1 >= rule.right_side.size()) {
		// if next isn't exist, use lookahead
		lookahead = c->lookahead();
	}else {
		// use first(next)
		auto &next = rule.right_side[idx_after_cursor + 1];
		if (next.termnon->isTerminal() == true)
			lookahead.insert(next);
		else
			lookahead = first_map.at(next);
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
		cout << itr.first.toString() << " ==> " << itr.second->name() << endl;
	}
}

static void _makeNextTransitionInfoMap (
		const vector<shared_ptr<Configuration>> &configs,
		unordered_map<RuleElem, vector<shared_ptr<Configuration>>, RuleElem::hash> &result) {

	for (auto &c : configs) {
		if (c->idx_after_cursor() >= c->rule().right_side.size())
			continue;

		auto &re = c->rule().right_side[c->idx_after_cursor()];

		result[re].push_back(
				make_shared<Configuration>(
					c->left_side(),
					c->rule(),
					c->idx_after_cursor() + 1,
					c->lookahead()));
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
		if (state_c->left_side()              != other_c->left_side()             ||
			state_c->idx_after_cursor()       != other_c->idx_after_cursor()      ||
			state_c->rule().right_side.size() != other_c->rule().right_side.size())
			return false;

		auto &state_r = state_c->rule();
		auto &other_r = other_c->rule();
		for (int ri = 0; ri < state_r.right_side.size(); ++ri) {
			if (state_r.right_side[ri].termnon != other_r.right_side[ri].termnon)
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

static shared_ptr<State> _findState(
		const vector<shared_ptr<State>> &states,
		shared_ptr<State> new_state) {
	
	for (int si = 0; si < states.size(); ++si) {
		auto &s = states[si];
		if (s == null)
			continue;

		if (_areStatesEqual(s, new_state) == true)
			return s;
	}

	return null;
}

static void _addStates(
	const vector<shared_ptr<Nonterminal>> &symbols,
	const FirstMap &first_map,
	shared_ptr<State> base,
	vector<shared_ptr<State>> &all_states,
	set<State*> &history) {

	if (history.find(base.get()) != history.end())
		return;
	history.insert(base.get());

	// make next_map
	unordered_map<RuleElem, vector<shared_ptr<Configuration>>, RuleElem::hash> next_map;
	_makeNextTransitionInfoMap(base->transited_configs(), next_map);
	_makeNextTransitionInfoMap(base->closures(), next_map);

	// make new state
	for (auto &itr : next_map) {
		auto new_state = State::makeState(symbols, first_map, itr.second);

		auto old_one = _findState(all_states, new_state);
		if (old_one != null)
			// merge
			_mergeStates_configs(old_one->transited_configs(), new_state->transited_configs());
			_mergeStates_configs(old_one->closures()         , new_state->closures()         );

			base->transition_map()[itr.first] = old_one;
		else {
			base->transition_map()[itr.first] = new_state;
			all_states.push_back(new_state);
		}
	}

}

shared_ptr<ParsingTable> ParsingTableGenerator::generateTable () {

	if (start_symbol_ == null) {
		//TODO err: you have to set start_symbol
		return null;
	}

    // make rule elements
    rule_elements_.clear();
    for (auto &non :  symbols_) {
        rule_elements_.insert(RuleElem(non, RuleElem::ANY));
        for (auto &rule : non->rules) {
            for (auto &re : rule.right_side)
                rule_elements_.insert(re);
        }
    }

	// make first map
	FirstMap first_map;
	for (auto &re : rule_elements_) {
		auto &first = first_map[re];
		_findFirst(re, first);
	}


	// make s_prime
	auto s_prime = make_shared<Nonterminal>("S\'");
	s_prime->rules.push_back(Rule(s_prime, { RuleElem(start_symbol_, RuleElem::ANY) }));

	// make start state
	set<RuleElem> start_lookahead{ RuleElem(null, RuleElem::ANY) };
	vector<shared_ptr<Configuration>> start_configs {
		make_shared<Configuration>(s_prime, s_prime->rules[0], 0, start_lookahead)
	};

	auto start_state = State::makeState(symbols_, first_map, start_configs);
	states_.push_back(start_state);

	// add states
	set<State*> history;
	for (int si = 0; si < states_.size(); ++si) {
		auto &s = states_[si];
		s->name("State " + to_string(si));
		_addStates(symbols_, first_map, s, states_, history);
	}

	// merge states
	_mergeStates(states_, states_);

	// print states
	for (auto &s : states_) {
		cout << "################################" << endl;
		s->print();
	}

	// make parsing table
	return make_shared<ParsingTable>(symbols_, s_prime, states_);
}


ParsingTable::ActionInfo::ActionInfo ()
:action(ActionInfo::NONE),
 idx(-1) {
}

ParsingTable::ActionInfo::ActionInfo (Action action, int idx)
:action(action),
 idx(idx) {

}

static void _makeReduceTable (
		const unordered_map<const Rule*, int> &rule_idx_map,
		const shared_ptr<Nonterminal> &start_symbol,
		const vector<shared_ptr<Configuration>> &configs,
		unordered_map<RuleElem, ParsingTable::ActionInfo, RuleElem::hash> &action_info_map) {

	for (auto &c : configs) {
		auto &rule = c->rule();
		if (c->idx_after_cursor() < rule.right_side.size())
			continue;

		int rule_idx = rule_idx_map.at(&rule);
		for (auto &re : c->lookahead()) {
			if (c->left_side() == start_symbol && re.termnon == null) {
				action_info_map[re] = ParsingTable::ActionInfo(
						ParsingTable::ActionInfo::ACCEPT, rule_idx);
			}else {
				action_info_map[re] = ParsingTable::ActionInfo(
						ParsingTable::ActionInfo::REDUCE, rule_idx);
			}
		}
	}
}

static void _makeTransitionTable(
		const unordered_map<RuleElem, shared_ptr<State>, RuleElem::hash> &transition_map,
		const unordered_map<shared_ptr<State>, int> &state_idx_map,
		unordered_map<RuleElem, ParsingTable::ActionInfo, RuleElem::hash> &action_info_map) {

	for (auto &itr : transition_map) {
		auto &re    = itr.first;
		auto &state = itr.second;

		if (re.termnon->isTerminal() == true) {
			action_info_map[re] =
				ParsingTable::ActionInfo(
					ParsingTable::ActionInfo::SHIFT, state_idx_map.at(state));
		}else {
			action_info_map[re] =
				ParsingTable::ActionInfo(
					ParsingTable::ActionInfo::GOTO, state_idx_map.at(state));
		}
	}
}

ParsingTable::ParsingTable(
		const vector<shared_ptr<Nonterminal>> &symbols,
		const shared_ptr<Nonterminal> &start_symbol,
		const vector<shared_ptr<State>> &states) {

	unordered_map<shared_ptr<State>, int> state_idx_map;
	for (int si = 0; si < states.size(); ++si)
		state_idx_map[states[si]] = si;

	// rules
	unordered_map<const Rule*, int> rule_idx_map;
	for (auto &r : start_symbol->rules) {
		rule_idx_map[&r] = rules_.size();
		rules_.push_back(&r);
	}
	for (auto &non : symbols) {
		for (auto &r : non->rules) {
			rule_idx_map[&r] = rules_.size();
			rules_.push_back(&r);
		}
	}

	// make table
	action_info_map_list_.resize(states.size());
	for (int si = 0; si < states.size(); ++si) {
		auto &s = states[si];
		auto &action_info_map = action_info_map_list_[si];

		// make about reduce
		_makeReduceTable(rule_idx_map, start_symbol, s->transited_configs(), action_info_map);
		_makeReduceTable(rule_idx_map, start_symbol, s->closures()         , action_info_map);

		// transition
		_makeTransitionTable(s->transition_map(), state_idx_map, action_info_map);
	}
}

static string _actionInfoToString (ParsingTable::ActionInfo info) {
    switch (info.action) {
        case ParsingTable::ActionInfo::NONE:
            return "none";
        case ParsingTable::ActionInfo::SHIFT:
            return "s" + to_string(info.idx);
        case ParsingTable::ActionInfo::REDUCE:
            return "r" + to_string(info.idx);
        case ParsingTable::ActionInfo::GOTO:
            return "go" + to_string(info.idx);
        case ParsingTable::ActionInfo::ACCEPT:
            return "acc";
    }
}

static bool _sortFuncForRuleElemByString (const RuleElem &a, const RuleElem &b) {
    return (a.toString() < b.toString());
}

void ParsingTable::print () const {
    cout << "##### Rules" << endl;
    for (int ri=0; ri<rules_.size(); ++ri) {
        auto &r = rules_[ri];
        cout << "# Rule " << ri << " : ";
        r->print();
    }


    cout << "##### Table" << endl;
    set<RuleElem> rule_elem_set;
    for (auto &action_map : action_info_map_list_) {
        for (auto &itr : action_map)
            rule_elem_set.insert(itr.first);
    }

    // make field string length map
    unordered_map<RuleElem, int, RuleElem::hash> field_str_length_map;
    for (auto &re : rule_elem_set)
        field_str_length_map[re] = re.toString().size();

    for (auto &action_map : action_info_map_list_) {
        for (auto &itr : action_map) {
            auto &field_str_length = field_str_length_map[itr.first];
            auto action_str = _actionInfoToString(itr.second);
            field_str_length =
                    (action_str.size() > field_str_length)?
                        action_str.size() : field_str_length;
        }
    }


    // print
    vector<RuleElem> rule_elem_vec; // terminal first
    for (auto &re : rule_elem_set) {
        if (re.termnon == null || re.termnon->isTerminal() == true)
            rule_elem_vec.push_back(re);
    }
    sort(rule_elem_vec.begin(), rule_elem_vec.end(), _sortFuncForRuleElemByString);

    vector<RuleElem> rule_elem_vec_for_non; // terminal first
    for (auto &re : rule_elem_set) {
        if (re.termnon != null && re.termnon->isTerminal() == false)
            rule_elem_vec_for_non.push_back(re);
    }
    sort(   rule_elem_vec_for_non.begin(),
            rule_elem_vec_for_non.end  (),
            _sortFuncForRuleElemByString);
    rule_elem_vec.insert(
            rule_elem_vec.end(),
            rule_elem_vec_for_non.begin(),
            rule_elem_vec_for_non.end());

    
    // field
    int field_length = 0;
    cout << "   " << " | "; field_length += 6;
    for (auto &re : rule_elem_vec) {
        auto length = field_str_length_map[re];
        cout << setfill(' ') << setw(length) << re.toString() << " | ";
        field_length += length + 3;
    }
    cout << endl;
    for (int fi=0; fi<field_length; ++fi)
        cout << "-";
    cout << endl;

    // content
    for (int ai=0; ai<action_info_map_list_.size(); ++ai) {
        cout << setfill(' ') << setw(3) << ai << " | ";
        auto &action_map = action_info_map_list_[ai];
        for (auto &re : rule_elem_vec) {
            auto length = field_str_length_map[re];
            if (action_map.find(re) == action_map.end())
                cout << setfill(' ') << setw(length) << " " << " | ";
            else {
                cout << setfill(' ') << setw(length)
                        << _actionInfoToString(action_map.at(re)) << " | ";
            }
        }
        cout << endl;
    }

}

}

