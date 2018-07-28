#include "node.h"

#include "./defines.h"

#include <iostream>


namespace paw_print {

using std::cout;
using std::endl;
using std::dynamic_pointer_cast;


shared_ptr<Nonterminal> TerminalBase::start_;


static int _parse_step (
        const char *text,
        const vector<Token> &tokens,
        int token_idx,
        const shared_ptr<Nonterminal> &non,
        shared_ptr<Node> &node);


// return next token idx
static int _checkRuleAndSetChildNode (
        const char *text,
        const vector<Token> &tokens,
        int token_idx,
        const shared_ptr<Nonterminal> &non,
        int rule_idx,
        shared_ptr<Node> &node) {
    
    int pre_indent = 0;

    node->setRuleAndPrepareChildren(rule_idx);

    auto &rule = non->rules[rule_idx];
    for (int ri=0; ri<rule.size(); ++ri) {
        auto &token = tokens[token_idx];

        auto &re = rule[ri];
        auto child_node = make_shared<Node>(re.termnon, 0, token_idx);
        node->setChild(ri, child_node);

        if (re.termnon->isTerminal()) {
            auto term = dynamic_pointer_cast<Terminal>(re.termnon);
            if (term->type != token.type)
                return token_idx;

            switch (re.indent_type) {
                case RuleElem::ANY: break;
                case RuleElem::SAME:
                    if (token.indent != pre_indent)
                        return token_idx;
                    break;
                case RuleElem::BIGGER:
                    if (token.indent - pre_indent != 4)
                        return token_idx;
                    break;
            }
            token_idx += 1;
        }else {
            auto non = dynamic_pointer_cast<Nonterminal>(re.termnon);
            auto next_token_idx = _parse_step(text, tokens, token_idx, non, child_node);
            if (next_token_idx == token_idx)
                return token_idx;
            token_idx = next_token_idx;
        }

        pre_indent = token.indent;
    }

    return token_idx;
}

// return next token idx
static int _parse_step (
        const char *text,
        const vector<Token> &tokens,
        int token_idx,
        const shared_ptr<Nonterminal> &non,
        shared_ptr<Node> &node) {

    for (int ri=0; ri<non->rules.size(); ++ri) {
        auto next_token_idx = _checkRuleAndSetChildNode(text, tokens, token_idx, non, ri, node); 
        if (next_token_idx >= tokens.size())
            return next_token_idx;

        token_idx = next_token_idx;
    }

    return token_idx;
}

shared_ptr<Node> Node::parse (const char *text, const vector<Token> &tokens) {
    shared_ptr<Node> node = make_shared<Node>(TerminalBase::start(), 0, 0);
    auto next_token_idx = _parse_step(text, tokens, 0, TerminalBase::start(), node);
    if (next_token_idx < tokens.size())
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
            RuleElem(non_map, RuleElem::ANY ),
            RuleElem(non_kv , RuleElem::SAME),
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

Node::Node (const shared_ptr<TerminalBase> &termnon, int rule_idx, int token_idx)
:termnon_(termnon),
 rule_idx_(rule_idx),
 token_idx_(token_idx) {

    setRuleAndPrepareChildren(rule_idx);
}

void Node::setRuleAndPrepareChildren (int rule_idx) {
    rule_idx_ = rule_idx;

    if (termnon_->isTerminal() == true)
        return;

    // make children
    auto non = dynamic_pointer_cast<Nonterminal>(termnon_);
    auto &rule = non->rules[rule_idx];
    children_.resize(rule.size());
    for (int ci=0; ci<children_.size(); ++ci) {
        children_[ci] = null;
    }
}

}
