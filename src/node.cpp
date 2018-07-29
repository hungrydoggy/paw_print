#include "node.h"

#include "./defines.h"

#include <iostream>
#include <set>


namespace paw_print {

using std::cout;
using std::endl;
using std::dynamic_pointer_cast;
using std::set;

using RuleHistoryByTokenIdx = unordered_map<int, set<unsigned int>>;


shared_ptr<Nonterminal> TerminalBase::start_;

unsigned int Nonterminal::next_no_ = 0;



static bool _checkNextRule(
		RuleHistoryByTokenIdx &history_map,
		const char *text,
		const vector<Token> &tokens,
		Node *node);



static unsigned int _makeRuleKey(unsigned int non_no, int rule_idx) {
	return (non_no << 16) | ((unsigned int)rule_idx);
}

static bool _isInHistory(
		RuleHistoryByTokenIdx &history_map,
		int token_idx,
		unsigned int rule_key) {

	if (history_map.find(token_idx) == history_map.end())
		return false;

	auto &history = history_map[token_idx];
	return history.find(rule_key) != history.end();
}

static void _addToHistory(
		RuleHistoryByTokenIdx &history_map,
		int token_idx,
		unsigned int rule_key) {

	auto &history = history_map[token_idx];
	history.insert(rule_key);
}

static bool _onEndOfNode(
		RuleHistoryByTokenIdx &history_map,
        const char *text,
        const vector<Token> &tokens,
        Node *node) {

    // find out
    if (node->token_idx() + 1 >= tokens.size())
        return true;

    
    // try more
    return _checkNextRule(history_map, text, tokens, node->findPrePriorityNode());
}

static bool _checkNode (
		RuleHistoryByTokenIdx &history_map,
        const char *text,
        const vector<Token> &tokens,
        int token_idx,
        Node *node) {

	if (token_idx >= tokens.size())
		return _checkNextRule(history_map, text, tokens, node);

    node->token_idx(token_idx);
	auto &token = tokens[token_idx];


	// check indent
	int pre_indent = 0;
	auto pre_brother = node->findPreBrother();
	if (pre_brother != null)
		pre_indent = tokens[pre_brother->token_idx()].indent;

	bool is_indent_ok = true;
	switch (node->indent_type()) {
	case RuleElem::ANY: break;
	case RuleElem::SAME:
		if (token.indent != pre_indent)
			is_indent_ok = false;
		break;
	case RuleElem::SMALLER:
		if (token.indent - pre_indent != -4)
			is_indent_ok = false;
		break;
	case RuleElem::BIGGER:
		if (token.indent - pre_indent != 4)
			is_indent_ok = false;
		break;
	}
	if (is_indent_ok == false) {
		auto non = dynamic_pointer_cast<Nonterminal>(
			(node->termnon()->isTerminal()) ?
			node->parent()->termnon() : node->termnon());
		auto rule_idx = (node->termnon()->isTerminal()) ?
				node->parent()->rule_idx() : node->rule_idx();
		_addToHistory(
				history_map,
				node->parent()->children()[0]->token_idx(),
				_makeRuleKey(non->no(), rule_idx));
		return _checkNextRule(history_map, text, tokens, node->findPrePriorityNode());
	}


	// check terminal
    if (node->termnon()->isTerminal() == true) {
        auto term = dynamic_pointer_cast<Terminal>(node->termnon());
        if (term->type != token.type) {
            // not correct terminal
			auto non = dynamic_pointer_cast<Nonterminal>(node->parent()->termnon());
			_addToHistory(
					history_map,
					node->token_idx(),
					_makeRuleKey(non->no(), node->parent()->rule_idx()));
            return _checkNextRule(history_map, text, tokens, node->parent());
        }


        // correct terminal
        //to next node
        auto next_node = node->findNextLeafNode();
        if (next_node == null)
            return _onEndOfNode(history_map, text, tokens, node);

        return _checkNode(
				history_map,
                text,
                tokens,
                node->token_idx() + 1,
                next_node);
    }
    

    // nonterminal
	return _checkNextRule(history_map, text, tokens, node);
}

static bool _checkNextRule(
	RuleHistoryByTokenIdx &history_map,
	const char *text,
	const vector<Token> &tokens,
	Node *node) {

	if (node == null)
		return false;

	// check has next rule
	auto non = dynamic_pointer_cast<Nonterminal>(node->termnon());
	auto next_rule_idx = node->rule_idx() + 1;
	auto rule_key = _makeRuleKey(non->no(), next_rule_idx);
	while (_isInHistory(history_map, node->token_idx(), rule_key) == true) {
		++next_rule_idx;
		rule_key = _makeRuleKey(non->no(), next_rule_idx);
	}

    if (next_rule_idx >= non->rules.size()) {
        // no next rule

		// try more
		auto pre_node = node->findPrePriorityNode();
		if (pre_node == null) // semantic error
			return false;
        return _checkNextRule(history_map, text, tokens, pre_node);
    }


    // next rule
    node->setRuleAndPrepareChildren(next_rule_idx);

    auto &rules = non->rules[next_rule_idx];
    for (int ri=0; ri<rules.size(); ++ri) {
        auto &r = rules[ri];
        auto child = make_shared<Node>(r.termnon, r.indent_type);
        node->setChild(ri, child);
    }

    return _checkNode(
			history_map,
            text,
            tokens,
            node->token_idx(),
            node->children()[0].get());
}


shared_ptr<Node> Node::parse (const char *text, const vector<Token> &tokens) {

	RuleHistoryByTokenIdx history_map;

    shared_ptr<Node> node = make_shared<Node>(TerminalBase::start(), RuleElem::IndentType::ANY);

    auto is_ok = _checkNode(history_map, text, tokens, 0, node.get());
    if (is_ok == false)
        return null;

    return node;
}

void TerminalBase::_init () {
    static bool is_inited = false;
    if (is_inited == true)
        return;
    is_inited = true;


    auto term_int         = make_shared<Terminal>("int"        , Token::INT        );
    auto term_double      = make_shared<Terminal>("double"     , Token::DOUBLE     );
    auto term_string      = make_shared<Terminal>("string"     , Token::STRING     );
    auto term_colon       = make_shared<Terminal>("colon"      , Token::COLON      );
    auto term_comma       = make_shared<Terminal>("comma"      , Token::COMMA      );
	auto term_curly_open  = make_shared<Terminal>("curly_open" , Token::CURLY_OPEN );
	auto term_curly_close = make_shared<Terminal>("curly_close", Token::CURLY_CLOSE);

    auto non_kv  = make_shared<Nonterminal>("KV" );
    auto non_map = make_shared<Nonterminal>("MAP");

	auto non_kv_for_blocked_content = make_shared<Nonterminal>("KV_FOR_BLOCKED_CONTENTS");
	auto non_map_blocked_content    = make_shared<Nonterminal>("MAP_BLOCKED_CONTENTS"   );

	auto non_sequence = make_shared<Nonterminal>("SEQUENCE");

    auto non_node     = make_shared<Nonterminal>("NODE");

	// KV
    non_kv->rules.push_back({
            RuleElem(non_node  , RuleElem::ANY   ),
            RuleElem(term_colon, RuleElem::SAME  ),
            RuleElem(non_node  , RuleElem::BIGGER),
        });

	// MAP
    non_map->rules.push_back({ 
			RuleElem(term_curly_open , RuleElem::ANY),
			RuleElem(term_curly_close, RuleElem::ANY),
        });
    non_map->rules.push_back({ 
			RuleElem(term_curly_open        , RuleElem::ANY),
			RuleElem(non_map_blocked_content, RuleElem::ANY),
			RuleElem(term_curly_close       , RuleElem::ANY),
        });
    non_map->rules.push_back({ 
            RuleElem(non_kv, RuleElem::ANY),
        });
    non_map->rules.push_back({ 
            RuleElem(non_kv , RuleElem::ANY ),
            RuleElem(non_map, RuleElem::SAME),
        });
	
	// KV_FOR_BLOCKED_CONTENT
    non_kv->rules.push_back({
            RuleElem(non_node  , RuleElem::ANY),
            RuleElem(term_colon, RuleElem::ANY),
            RuleElem(non_node  , RuleElem::ANY),
        });

	// MAP_BLOCKED_CONTENT
	non_map_blocked_content->rules.push_back({
            RuleElem(non_kv_for_blocked_content, RuleElem::ANY),
		});
	non_map_blocked_content->rules.push_back({
            RuleElem(non_kv_for_blocked_content, RuleElem::ANY),
			RuleElem(term_comma                , RuleElem::ANY),
			RuleElem(non_map_blocked_content   , RuleElem::ANY),
		});
	
	// SEQUENCE
    non_sequence->rules.push_back({ 
            RuleElem(non_kv, RuleElem::ANY),
        });
	non_sequence->rules.push_back({
            RuleElem(non_kv , RuleElem::ANY ),
            RuleElem(non_map, RuleElem::SAME),
        });
    

    non_node->rules.push_back({ RuleElem(term_int    , RuleElem::ANY), });
    non_node->rules.push_back({ RuleElem(term_double , RuleElem::ANY), });
    non_node->rules.push_back({ RuleElem(term_string , RuleElem::ANY), });
    non_node->rules.push_back({ RuleElem(non_map     , RuleElem::ANY), });
            //RuleElem(non_sequence, RuleElem::ANY),

    start_ = make_shared<Nonterminal>("S");
    start_->rules.push_back({ RuleElem(non_node, RuleElem::ANY) });
}


TerminalBase::TerminalBase (const string &name)
:name(name) {
}

Terminal::Terminal (const string &name, Token::Type type)
:TerminalBase(name),
 type(type) {
}

Nonterminal::Nonterminal (const string &name)
:TerminalBase(name) {
	no_ = next_no_;
	++next_no_;
}

Node::Node (const shared_ptr<TerminalBase> &termnon, RuleElem::IndentType indent_type)
:parent_(null),
 termnon_(termnon),
 indent_type_(indent_type),
 rule_idx_(-1),
 token_idx_(-1) {

}

void Node::setRuleAndPrepareChildren (int rule_idx) {
    rule_idx_ = rule_idx;

    if (termnon_->isTerminal() == true)
        return;

    // make children
    auto non = dynamic_pointer_cast<Nonterminal>(termnon_);
    auto &rule = non->rules[rule_idx];
    children_.resize(rule.size());
}

int Node::_findChild (Node *child) {
    for (int ci=0; ci<children_.size(); ++ci) {
        if (child == children_[ci].get())
            return ci;
    }
    
    return -1;
}

Node* Node::findPreBrother() {
	if (parent_ == null)
		return null;

	int pre_bro_idx = parent_->_findChild(this) - 1;
	if (pre_bro_idx < 0)
		return null;

	return parent_->children_[pre_bro_idx].get();
}

Node* Node::findPrePriorityNode () {
    if (parent_ == null)
        return null;

	auto pre_brother = findPreBrother();
	if (pre_brother == null)
		return parent_;

	if (pre_brother->termnon()->isTerminal() == true)
		return parent_;

	auto node = pre_brother;
    while (true) {
        if (node->children_.size() <= 0)
            return node;

        auto child = node->children_[node->children_.size() - 1];
        if (child == null || child->termnon()->isTerminal() == true)
            return node;

        node = child.get();
    }
}

Node* Node::findNextLeafNode () {
    if (parent_ == null)
        return null;

    auto next_bro_idx = parent_->_findChild(this) + 1;
    if (next_bro_idx >= parent_->children_.size())
        return parent_->findNextLeafNode();

    auto node = parent_->children_[next_bro_idx].get();
    while (true) {
        if (node->children_.size() <= 0)
            return node;

        auto child = node->children_[0];
        if (child == null)
            return node;

        node = child.get();
    }
}

}
