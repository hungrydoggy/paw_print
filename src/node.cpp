#include "node.h"

#include "./defines.h"

#include <iostream>


namespace paw_print {

using std::cout;
using std::endl;
using std::dynamic_pointer_cast;


shared_ptr<Nonterminal> TerminalBase::start_;



static bool _checkNextRule (
        const char *text, const vector<Token> &tokens, Node *node);

static bool _onEndOfNode(
        const char *text,
        const vector<Token> &tokens,
        Node *node) {

    // find out
    if (node->token_idx() + 1 >= tokens.size())
        return true;

    
    // try more
	if (node->parent() == null)
		return false;

    return _checkNextRule(text, tokens, node->parent());
}

static bool _checkNode (
        const char *text,
        const vector<Token> &tokens,
        int token_idx,
        Node *node) {

	if (token_idx >= tokens.size())
		return _checkNextRule(text, tokens, node->parent());

    node->token_idx(token_idx);
	auto &token = tokens[token_idx];


	// check indent
	int pre_indent = 0;
	auto pre_brother = node->findPreBrother();
	if (pre_brother != null)
		pre_indent = tokens[pre_brother->token_idx()].indent;

	switch (node->indent_type()) {
	case RuleElem::ANY: break;
	case RuleElem::SAME:
		if (token.indent != pre_indent)
			return _checkNextRule(text, tokens, node->parent());
		break;
	case RuleElem::SMALLER:
		if (token.indent - pre_indent != -4)
			return _checkNextRule(text, tokens, node->parent());
		break;
	case RuleElem::BIGGER:
		if (token.indent - pre_indent != 4)
			return _checkNextRule(text, tokens, node->parent());
		break;
	}


	// check terminal
    if (node->termnon()->isTerminal() == true) {
        auto term = dynamic_pointer_cast<Terminal>(node->termnon());
        if (term->type != token.type) {
            // not correct terminal
            return _checkNextRule(text, tokens, node->parent());
        }


        // correct terminal
        //to next node
        auto next_node = node->findNextLeafNode();
        if (next_node == null)
            return _onEndOfNode(text, tokens, node);

        return _checkNode(
                text,
                tokens,
                node->token_idx() + 1,
                next_node);
    }
    

    // nonterminal
	return _checkNextRule(text, tokens, node);
}

static bool _checkNextRule (
        const char *text, const vector<Token> &tokens, Node *node) {

    if (node == null)
        return false;

    auto non = dynamic_pointer_cast<Nonterminal>(node->termnon());
    auto next_rule_idx = node->rule_idx() + 1;
    if (next_rule_idx >= non->rules.size()) {
        // no next rule

		// try more
		auto pre_node = node->findPrePriorityNode();
		if (pre_node == null) // semantic error
			return false;
        return _checkNextRule(text, tokens, pre_node);
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
            text,
            tokens,
            node->token_idx(),
            node->children()[0].get());
}

shared_ptr<Node> Node::parse (const char *text, const vector<Token> &tokens) {
    shared_ptr<Node> node = make_shared<Node>(TerminalBase::start(), RuleElem::IndentType::ANY);

    auto is_ok = _checkNode(text, tokens, 0, node.get());
    if (is_ok == false)
        return null;

    return node;
}

void TerminalBase::_init () {
    static bool is_inited = false;
    if (is_inited == true)
        return;
    is_inited = true;


    auto term_int    = make_shared<Terminal>("int"   , Token::INT   );
    auto term_double = make_shared<Terminal>("double", Token::DOUBLE);
    auto term_string = make_shared<Terminal>("string", Token::STRING);
    auto term_colon  = make_shared<Terminal>("colon" , Token::COLON );

    auto non_kv   = make_shared<Nonterminal>("KV"  );
    auto non_map  = make_shared<Nonterminal>("MAP" );
    auto non_node = make_shared<Nonterminal>("NODE");

    non_kv->rules.push_back({
            RuleElem(non_node  , RuleElem::ANY   ),
            RuleElem(term_colon, RuleElem::SAME  ),
            RuleElem(non_node  , RuleElem::BIGGER),
        });


    non_map->rules.push_back({ 
            RuleElem(non_kv, RuleElem::ANY),
        });
    non_map->rules.push_back({ 
            RuleElem(non_kv , RuleElem::ANY ),
            RuleElem(non_map, RuleElem::SAME),
        });

    //auto non_sequence = make_shared<Nonterminal>("SEQUENCE");
    

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
