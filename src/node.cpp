#include "node.h"

#include <iostream>


namespace paw_print {

using std::cout;
using std::endl;
using std::dynamic_pointer_cast;


shared_ptr<Nonterminal> Node::start;


static bool _parse_step (
        const char *text,
        const vector<Token> &tokens,
        int idx,
        const shared_ptr<Nonterminal> &non);


static bool _checkRule (
        const char *text,
        const vector<Token> &tokens,
        int idx,
        const shared_ptr<Nonterminal> &non,
        int rule_idx) {
    
    int pre_indent = 0;

    auto &rule = non->rules[rule_idx];
    for (int ri=0; ri<rule.size(); ++ri) {
        int token_idx = idx + ri;
        auto &token = tokens[token_idx];

        auto &re = rule[ri];
        if (re.node->isTerminal()) {
            auto term = dynamic_pointer_cast<Terminal>(re.node);
            if (term->type != token.type)
                return false;

            switch (re.indent_type) {
                case RuleElem::ANY: break;
                case RuleElem::SAME:
                    if (token.indent != pre_indent)
                        return false;
                    break;
                case RuleElem::BIGGER:
                    if (token.indent - pre_indent != 4)
                        return false;
                    break;
            }
        }else {
            auto non = dynamic_pointer_cast<Nonterminal>(re.node);
            auto is_ok = _parse_step(text, tokens, token_idx, non);
            if (is_ok == false)
                return false;
        }

        pre_indent = token.indent;
    }

    return true;
}

static bool _parse_step (
        const char *text,
        const vector<Token> &tokens,
        int idx,
        const shared_ptr<Nonterminal> &non) {

    for (int ri=0; ri<non->rules.size(); ++ri) {
        auto is_ok = _checkRule(text, tokens, idx, non, ri); 
        if (is_ok == true) {
            //TODO
            return true;
        }
    }

    return false;
}

shared_ptr<Node> Node::parse (const char *text, const vector<Token> &tokens) {
    _init();

    auto is_ok = _parse_step(text, tokens, 0, start);
    cout << is_ok << endl;

    return 0;
}

void Node::_init () {
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

    start = make_shared<Nonterminal>("S");
    start->rules.push_back({ RuleElem(non_node, RuleElem::ANY) });
}


Node::Node (const string &name)
:name(name) {
}

Terminal::Terminal (const string &name, Token::Type type)
:Node(name),
 type(type) {
}

Nonterminal::Nonterminal (const string &name)
:Node(name) {
}


}
