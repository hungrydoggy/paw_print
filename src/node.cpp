#include "node.h"

#include "./defines.h"

#include <iostream>
#include <set>


namespace parse_table {

using std::cout;
using std::endl;
using std::dynamic_pointer_cast;
using std::set;

using RuleHistoryByTokenIdx = unordered_map<int, set<unsigned int>>;


shared_ptr<Nonterminal> TerminalBase::start_;

unsigned int Nonterminal::next_no_ = 0;



Rule::Rule (
        const shared_ptr<Nonterminal> &left_side,
        const vector<shared_ptr<TerminalBase>> &right_side)
:left_side(left_side),
 right_side(right_side) {
}

void Rule::print () const {
    cout << left_side->name << " -> ";
    for (auto &termnon : right_side) {
        cout << termnon->name << " ";
    }
    cout << endl;
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

Node::Node (const shared_ptr<TerminalBase> &termnon)
:parent_(null),
 termnon_(termnon),
 rule_idx_(-1),
 token_idx_(-1) {

}

void Node::setRuleAndPrepareChildren (int rule_idx) {
    rule_idx_ = rule_idx;

    if (termnon_->isTerminal() == true)
        return;

    // make children
    auto non = dynamic_pointer_cast<Nonterminal>(termnon_);
    auto &rule = non->rules[rule_idx].right_side;
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
